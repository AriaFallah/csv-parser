#ifndef ARIA_CSV_H
#define ARIA_CSV_H

#include <iostream>
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
      ~File() { m_file.close(); }
      auto get() -> std::ifstream& { return m_file; }
    private:
      std::ifstream m_file;
    };

    // Reads and parses lines from a csv file
    class CsvReader {
    private:
      // Overengineered CRLF handling
      enum class Term : char { CRLF = -2 };
      friend bool operator==(const Term& t, const char& c) {
        return c == t;
      }
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
    public:
      struct Config {
        Config(){}
        Config(char e, char d, char t): escape(e), delimiter(d), terminator(static_cast<Term>(t)) {}
        char escape = '"';
        char delimiter = ',';
        Term terminator = Term::CRLF;
      };

      explicit CsvReader(const std::string& filename, const Config& c = Config{})
        : m_file(filename), m_escape(c.escape), m_delimiter(c.delimiter), m_terminator(c.terminator)
      {
        m_fieldbuf.reserve(m_fieldbuf_cap);
        m_rowbuf.reserve(m_rowbuf_cap);
      }

      auto empty() -> bool {
        return m_state == State::EMPTY;
      }

      auto read_all() -> CSV {
        CSV csv;
        for (;;) {
          auto row = get_row();
          if (row.empty()) {
            break;
          }
          csv.push_back(row);
        }
        return csv;
      }

      auto get_row() -> std::vector<std::string> {
        if (empty()) return {};
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
                  handle_crlf(stream);
                  continue;
                }

                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_FIELD;
                } else if (c == m_delimiter) {
                  add_field();
                } else {
                  m_state = State::IN_FIELD;
                  add_char(c);
                }

                break;
              }

              case State::IN_FIELD: {
                if (c == m_terminator) {
                  handle_crlf(stream);
                  m_state = State::START_FIELD;
                  add_field();
                  return add_row();
                }

                if (c == m_delimiter) {
                  m_state = State::START_FIELD;
                  add_field();
                } else {
                  add_char(c);
                }

                break;
              }

              case State::IN_ESCAPED_FIELD: {
                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_ESCAPE;
                } else {
                  add_char(c);
                }

                break;
              }

              case State::IN_ESCAPED_ESCAPE: {
                if (c == m_terminator) {
                  handle_crlf(stream);
                  m_state = State::START_FIELD;
                  add_field();
                  return add_row();
                }

                if (c == m_escape) {
                  m_state = State::IN_ESCAPED_FIELD;
                  add_char(c);
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
            m_state = State::EMPTY;
            if (!m_fieldbuf.empty()) {
              add_field();
            }
            return add_row();
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
      static constexpr int m_inputbuf_cap = 1024;
      static constexpr int m_fieldbuf_cap = 1024;

      // Buffers
      std::string m_fieldbuf{};
      char m_inputbuf[m_inputbuf_cap]{};
      std::vector<std::string> m_rowbuf{};

      // Misc
      bool m_eof = false;
      size_t m_cursor = m_inputbuf_cap;
      size_t m_inputbuf_size = m_inputbuf_cap;

      auto handle_crlf(std::ifstream& s) -> void {
        if (m_terminator == Term::CRLF && s.peek() == '\n') {
          s.get();
        }
      }

      auto add_char(const char c) -> void {
        m_fieldbuf += c;
      }

      auto add_field() -> void {
        m_rowbuf.push_back(m_fieldbuf);
        m_fieldbuf.clear();
      }

      auto add_row() -> std::vector<std::string> {
        auto row(m_rowbuf);
        m_rowbuf.clear();
        return row;
      }
    };
  }
}
#endif
