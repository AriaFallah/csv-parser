#ifndef ARIA_CSV_H
#define ARIA_CSV_H

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace aria {
  namespace csv {
    using CSV = std::vector<std::vector<std::string>>;

    // RAII file class
    class File {
    public:
      explicit File(const std::string& filename): m_file(filename) {
        if (!m_file.good()) {
          throw std::runtime_error("There was an error opening this file!");
        }
      }
      ~File() {
        m_file.close();
      }
      std::ifstream& get() {
        return m_file;
      }
    private:
      std::ifstream m_file;
    };

    // Reads and parses lines from a csv file
    class CsvReader {
    private:
      // Overengineered CRLF handling
      enum class Term : char { CRLF = -2 };
      friend bool operator==(const char& c, const Term& t) {
        switch (t) {
          case Term::CRLF: {
            return c == '\r' || c == '\n';
          }
          default: {
            return static_cast<char>(t) == c;
          }
        }
      }
      friend bool operator!=(const char& c, const Term& t) {
        return !(c == t);
      }
    public:
      struct Config {
        Config(){}
        Config(char e, char d, char t)
          : escape(e),
            delimiter(d),
            terminator(static_cast<Term>(t))
        {}
        char escape = '"';
        char delimiter = ',';
        Term terminator = Term::CRLF;
      };

      explicit CsvReader(const std::string& filename, const Config& c = Config{})
        : m_file(filename),
          m_escape(c.escape),
          m_delimiter(c.delimiter),
          m_terminator(c.terminator)
      {
        m_fieldbuf.reserve(m_fieldbuf_cap);
        m_rowbuf.reserve(m_rowbuf_cap);
      }

      bool empty() {
        return m_state == State::EMPTY;
      }

      CSV read_all() {
        CSV csv;
        for (;;) {
          auto row = get_row();
          if (!row) {
            break;
          }
          csv.push_back(*row);
        }
        return csv;
      }

      std::vector<std::string>* get_row() {
        if (empty()) return nullptr;

        m_rowbuf.clear();
        std::ifstream& stream = m_file.get();

        for (;;) {
          if (m_cursor == m_inputbuf_size) {
            m_cursor = 0;
            stream.read(m_inputbuf, m_inputbuf_cap);

            if (stream.eof()) {
              m_eof = true;
              m_inputbuf_size = stream.gcount();
            }
          }

          while (m_cursor < m_inputbuf_size) {
            char c = m_inputbuf[m_cursor++];

            switch (m_state) {
              case State::START_FIELD: {
                if (c == m_terminator) {
                  handle_crlf(c);
                  add_field();
                  return &m_rowbuf;
                }

                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_FIELD;
                } else if (c == m_delimiter) {
                  add_field();
                } else {
                  m_state = State::IN_FIELD;
                  m_fieldbuf += c;
                }

                break;
              }

              case State::IN_FIELD: {
                if (c == m_terminator) {
                  handle_crlf(c);
                  m_state = State::START_FIELD;
                  add_field();
                  return &m_rowbuf;
                }

                if (c == m_delimiter) {
                  m_state = State::START_FIELD;
                  add_field();
                } else {
                  m_fieldbuf += c;
                }

                break;
              }

              case State::IN_ESCAPED_FIELD: {
                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_ESCAPE;
                } else {
                  m_fieldbuf += c;
                }

                break;
              }

              case State::IN_ESCAPED_ESCAPE: {
                if (c == m_terminator) {
                  handle_crlf(c);
                  m_state = State::START_FIELD;
                  add_field();
                  return &m_rowbuf;
                }

                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_FIELD;
                  m_fieldbuf += c;
                } else if (c == m_delimiter) {
                  m_state = State::START_FIELD;
                  add_field();
                } else {
                  throw std::runtime_error("CSV is malformed");
                }

                break;
              }

              case State::EMPTY: {
                throw std::runtime_error("How did this even happen?");
              }
            }
          }

          if (m_eof) {
            char c = m_inputbuf[m_cursor - 1]; // last read character
            if (!m_fieldbuf.empty() || c == m_delimiter) {
              add_field();
            }
            m_state = State::EMPTY;
            return m_rowbuf.empty() ? nullptr : &m_rowbuf;
          }
        }
      }
    private:
      // CSV state for state machine
      enum class State {
        START_FIELD,
        IN_FIELD,
        IN_ESCAPED_FIELD,
        IN_ESCAPED_ESCAPE,
        EMPTY
      };
      State m_state = State::START_FIELD;

      // Configurable attributes
      File m_file;
      char m_escape;
      char m_delimiter;
      Term m_terminator;

      // Buffer capacities
      static constexpr int m_rowbuf_cap = 50;
      static constexpr int m_fieldbuf_cap = 1024;
      static constexpr int m_inputbuf_cap = 1024 * 128;

      // Buffers
      std::string m_fieldbuf{};
      char m_inputbuf[m_inputbuf_cap]{};
      std::vector<std::string> m_rowbuf{};

      // Misc
      bool m_eof = false;
      size_t m_cursor = m_inputbuf_cap;
      size_t m_inputbuf_size = m_inputbuf_cap;

      void handle_crlf(const char c) {
        if (m_terminator != Term::CRLF || c != '\r') {
          return;
        }

        if (m_inputbuf[m_cursor] == '\n') {
          m_cursor++;
        }
      }

      void add_field() {
        m_rowbuf.push_back(m_fieldbuf);
        m_fieldbuf.clear();
      }
    };
  }
}
#endif
