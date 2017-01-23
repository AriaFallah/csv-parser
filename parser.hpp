#ifndef ARIA_CSV_H
#define ARIA_CSV_H

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace aria {
  namespace csv {
    const char ESCAPE = '"';
    const char DELIMITER = ',';
    const char TERMINATOR = '\n';
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
    public:
      explicit CsvReader(const std::string& filename)
        : m_file(filename), m_state(State::START_FIELD), m_bufstr(), m_bufvec()
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

          // Ignore carriage returns. Should fix this in future.
          if (m_state != State::IN_ESCAPED_FIELD && c == '\r') {
            continue;
          }

          switch (m_state) {
            case State::START_FIELD: {
              if (c == EOF) {
                m_state = State::EMPTY;
                return {};
              }
              if (c == TERMINATOR) {
                return add_row();
              }

              if (c == ESCAPE) {
                m_state = State::IN_ESCAPED_FIELD;
              } else if (c == DELIMITER) {
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
              if (c == TERMINATOR) {
                m_state = State::START_FIELD;
                add_field();
                return add_row();
              }

              if (c == DELIMITER) {
                m_state = State::START_FIELD;
                add_field();
              } else {
                add_char(c);
              }

              break;
            }

            case State::IN_ESCAPED_FIELD: {
              if (c == ESCAPE) {
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
              if (c == TERMINATOR) {
                m_state = State::START_FIELD;
                add_field();
                return add_row();
              }

              if (c == ESCAPE) {
                m_state = State::IN_ESCAPED_FIELD;
                add_char(c);
              } else if (c == DELIMITER) {
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
