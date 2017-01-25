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
      ~File() { m_file.close(); }
      auto get() -> std::ifstream& { return m_file; }
    private:
      std::ifstream m_file;
    };

    // Reads and parses lines from a csv file
    class CsvReader {
    private:
      // Overengineered CRLF handling
      enum class Term : short { CRLF = -1 };
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
      static auto make_term(char term) -> Term {
        return static_cast<Term>(term);
      }
    public:
      struct Config {
        Config(){}
        Config(char e, char d, char t): escape(e), delimiter(d), terminator(make_term(t)) {}
        char escape = '"';
        char delimiter = ',';
        Term terminator = Term::CRLF;
      };

      explicit CsvReader(const std::string& filename, const Config& c = Config{})
        : m_file(filename), m_state(State::START_FIELD), m_bufstr(),
          m_bufvec(), m_escape(c.escape), m_delimiter(c.delimiter), m_terminator(c.terminator)
      {
        // Reserve space for CSV upfront
        m_bufstr.reserve(10000);
        m_bufvec.reserve(10000);
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
          char c = stream.get();

          switch (m_state) {
            case State::START_FIELD: {
              if (c == EOF) {
                m_state = State::EMPTY;
                return {};
              }
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
              if (c == EOF) {
                m_state = State::EMPTY;
                add_field();
                return add_row();
              }
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
              if (c == EOF) {
                m_state = State::EMPTY;
                add_field();
                return add_row();
              }
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
      }
    private:
      enum class State : int { START_FIELD, IN_FIELD, IN_ESCAPED_FIELD, IN_ESCAPED_ESCAPE, EMPTY };

      File m_file;
      State m_state;
      std::string m_bufstr;
      std::vector<std::string> m_bufvec;

      // Configurable attributes
      char m_escape;
      char m_delimiter;
      Term m_terminator;

      auto handle_crlf(std::ifstream& s) -> void {
        if (m_terminator == Term::CRLF && s.peek() == '\n') {
          s.get();
        }
      }

      auto add_char(const char c) -> void {
        m_bufstr += c;
      }

      auto add_field() -> void {
        m_bufvec.push_back(m_bufstr);
        m_bufstr.clear();
      }

      auto add_row() -> std::vector<std::string> {
        auto row(m_bufvec);
        m_bufvec.clear();
        return row;
      }
    };
  }
}
#endif
