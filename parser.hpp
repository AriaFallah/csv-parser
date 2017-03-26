#ifndef ARIA_CSV_H
#define ARIA_CSV_H

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace aria {
  namespace csv {
    enum class Term : char { CRLF = -2 };
    using CSV = std::vector<std::vector<std::string>>;

    // Checking for '\n', '\r', and '\r\n' by default
    bool operator==(const char c, const Term t) {
      switch (t) {
        case Term::CRLF:
          return c == '\r' || c == '\n';
        default:
          return static_cast<char>(t) == c;
      }
    }

    bool operator!=(const char c, const Term t) {
      return !(c == t);
    }

    // Reads and parses lines from a csv file
    class CsvParser {
    public:
      enum class FieldType { DATA, ROW_END, CSV_END };

      // Config object for nicer CsvParser constructor API
      struct Config {
        constexpr Config() noexcept {};
        constexpr Config(char e, char d, char t) noexcept
          : escape(e),
            delimiter(d),
            terminator(static_cast<Term>(t))
        {}
        char escape = '"';
        char delimiter = ',';
        Term terminator = Term::CRLF;
      };

      struct Field {
        explicit Field(FieldType t): type(t) {}
        explicit Field(const std::string& str): type(FieldType::DATA), data(str) {}

        FieldType type;
        std::string data;
      };

      // Creates the CSV parser which by default, splits on commas,
      // uses quotes to escape, and handles CSV files that end in either
      // '\r', '\n', or '\r\n'.
      CsvParser(const std::string& filename, const Config& c = Config{})
        : m_file(filename),
          m_escape(c.escape),
          m_delimiter(c.delimiter),
          m_terminator(c.terminator)
      {
        // Reserve space upfront to improve performance
        m_fieldbuf.reserve(FIELDBUF_CAP);
      }

      // The parser is in the empty state when there are
      // no more tokens left to read from the input buffer
      bool empty() {
        return m_state == State::EMPTY;
      }

      // Reads a single row from the CSV
      Field next_field() {
        if (empty()) {
          return Field(FieldType::CSV_END);
        }

        // This loop runs until either the parser has
        // read an entire row or until there's no tokens left to read
        for (;;) {
          // If we're out of tokens to read return whatever's left in the
          // field and row buffers. If there's nothing left, return null.
          if (!top_token()) {
            m_state = State::EMPTY;
            return !m_fieldbuf.empty() ? Field(m_fieldbuf) : Field(FieldType::CSV_END);
          }

          // Parsing the CSV is done using a finite state machine
          char c = *pop_token();
          switch (m_state) {
            case State::START_OF_FIELD:
              if (c == m_terminator) {
                handle_crlf(c);
                return Field(FieldType::ROW_END);
              }

              if (c == m_escape) {
                m_state = State::IN_ESCAPED_FIELD;
              } else if (c == m_delimiter) {
                return Field(m_fieldbuf);
              } else {
                m_state = State::IN_FIELD;
                m_fieldbuf += c;
              }

              break;

            case State::IN_FIELD:
              if (c == m_terminator) {
                handle_crlf(c);
                m_state = State::END_OF_ROW;
                return Field(m_fieldbuf);
              }

              if (c == m_delimiter) {
                m_state = State::START_OF_FIELD;
                return Field(m_fieldbuf);
              } else {
                m_fieldbuf += c;
              }

              break;

            case State::IN_ESCAPED_FIELD:
              if (c == m_escape) {
                m_state = State::IN_ESCAPED_ESCAPE;
              } else {
                m_fieldbuf += c;
              }

              break;

            case State::IN_ESCAPED_ESCAPE:
              if (c == m_terminator) {
                handle_crlf(c);
                m_state = State::END_OF_ROW;
                return Field(m_fieldbuf);
              }

              if (c == m_escape) {
                m_state = State::IN_ESCAPED_FIELD;
                m_fieldbuf += c;
              } else if (c == m_delimiter) {
                m_state = State::START_OF_FIELD;
                return Field(m_fieldbuf);
              } else {
                throw std::runtime_error("CSV is malformed");
              }

              break;

            case State::END_OF_ROW:
              m_state = State::START_OF_FIELD;
              return Field(FieldType::ROW_END);

            case State::EMPTY:
              throw std::logic_error("You goofed");
          }
        }
      }
    private:
      // CSV state for state machine
      enum class State {
        START_OF_FIELD,
        IN_FIELD,
        IN_ESCAPED_FIELD,
        IN_ESCAPED_ESCAPE,
        END_OF_ROW,
        EMPTY
      };
      State m_state = State::START_OF_FIELD;

      // Configurable attributes
      std::ifstream m_file;
      char m_escape;
      char m_delimiter;
      Term m_terminator;

      // Buffer capacities
      static constexpr int FIELDBUF_CAP = 1024;
      static constexpr int INPUTBUF_CAP = 1024 * 128;

      // Buffers
      std::string m_fieldbuf{};
      char m_inputbuf[INPUTBUF_CAP]{};

      // Misc
      bool m_eof = false;
      size_t m_cursor = INPUTBUF_CAP;
      size_t m_inputbuf_size = INPUTBUF_CAP;

      // When the parser hits the end of a line it needs
      // to check the special case of '\r\n' as a terminator.
      // If it finds that the previous token was a '\r', and
      // the next token will be a '\n', it skips the '\n'.
      void handle_crlf(const char c) {
        if (m_terminator != Term::CRLF || c != '\r') {
          return;
        }

        char *token = top_token();
        if (token && *token == '\n') {
          m_cursor++;
        }
      }

      // Pulls the next token from the input buffer, but does not move
      // the cursor forward. If the stream is empty and the input buffer
      // is also empty return a nullptr.
      char* top_token() {
        // Return null if there's nothing left to read
        if (m_eof && m_cursor == m_inputbuf_size) {
          return nullptr;
        }

        // Refill the input buffer if it's been fully read
        if (m_cursor == m_inputbuf_size) {
          m_cursor = 0;
          m_file.read(m_inputbuf, INPUTBUF_CAP);

          // Indicate we hit end of file, and resize
          // input buffer to show that it's not at full capacity
          if (m_file.eof()) {
            m_eof = true;
            m_inputbuf_size = m_file.gcount();

            // Return null if there's nothing left to read
            if (m_inputbuf_size == 0) {
              return nullptr;
            }
          }
        }

        return &m_inputbuf[m_cursor];
      }

      // Same as top_token(), but it moves the cursor forward
      char* pop_token() {
        char *token = top_token();
        m_cursor++;
        return token;
      }
    };
  }
}
#endif
