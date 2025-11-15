#ifndef ARIA_CSV_H
#define ARIA_CSV_H

#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace aria {
namespace csv {
enum class Term { CRLF = -2 };
enum class FieldType { DATA, ROW_END, CSV_END };
using CSV = std::vector<std::vector<std::string>>;

// Checking for '\n', '\r', and '\r\n' by default
inline auto operator==(const char c, const Term t) -> bool {
  switch (t) {
  case Term::CRLF:
    return c == '\r' || c == '\n';
  default:
    return static_cast<char>(t) == c;
  }
}

inline auto operator!=(const char c, const Term t) -> bool { return !(c == t); }

// Wraps returned fields so we can also indicate
// that we hit row endings or the end of the csv itself
struct Field {
  explicit Field(FieldType t) : type(t) {}
  explicit Field(std::string &&str)
      : type(FieldType::DATA), data(std::move(str)) {}

  FieldType type;
  std::string data;
};

// Reads and parses lines from a csv file
class CsvParser {
private:
  // CSV state for state machine
  enum class State {
    START_OF_FIELD,
    IN_FIELD,
    IN_QUOTED_FIELD,
    IN_ESCAPED_QUOTE,
    END_OF_ROW,
    EMPTY
  };
  State m_state = State::START_OF_FIELD;

  // Configurable attributes
  char m_quote = '"';
  char m_delimiter = ',';
  Term m_terminator = Term::CRLF;
  std::istream *m_input;

  // Buffer capacities
  static constexpr int FIELDBUF_CAP = 1024;
  static constexpr int INPUTBUF_CAP = 1024 * 128;

  // Buffers
  std::string m_fieldbuf{};
  std::vector<char> m_inputbuf = std::vector<char>(INPUTBUF_CAP);

  // Misc
  bool m_eof = false;
  size_t m_cursor = 0;
  size_t m_bytes_read = 0;
  std::streamoff m_scanposition = 0;

public:
  // Delete copy constructor and assignment
  CsvParser(const CsvParser &) = delete;
  auto operator=(const CsvParser &) -> CsvParser & = delete;

  // Allow move operations
  CsvParser(CsvParser &&) = default;
  auto operator=(CsvParser &&) -> CsvParser & = default;

  // Creates the CSV parser which by default, splits on commas,
  // uses quotes to escape, and handles CSV files that end in either
  // '\r', '\n', or '\r\n'.
  explicit CsvParser(std::istream &input) : m_input(&input) {
    // Reserve space upfront to improve performance
    m_fieldbuf.reserve(FIELDBUF_CAP);
    if (!m_input->good()) {
      throw std::runtime_error("Something is wrong with input stream");
    }
  }

  // Change the quote character
  auto quote(char c) noexcept -> CsvParser && {
    m_quote = c;
    return std::move(*this);
  }

  // Change the delimiter character
  auto delimiter(char c) noexcept -> CsvParser && {
    m_delimiter = c;
    return std::move(*this);
  }

  // Change the terminator character
  auto terminator(char c) noexcept -> CsvParser && {
    m_terminator = static_cast<Term>(c);
    return std::move(*this);
  }

  // The parser is in the empty state when there are
  // no more tokens left to read from the input buffer
  auto empty() -> bool { return m_state == State::EMPTY; }

  // Not the actual position in the stream (its buffered) just the
  // position up to last availiable token
  auto position() const -> std::streamoff {
    return m_scanposition + static_cast<std::streamoff>(m_cursor);
  }

  // Reads a single field from the CSV
  auto next_field() -> Field {
    if (empty()) {
      return Field(FieldType::CSV_END);
    }
    m_fieldbuf.clear();

    // This loop runs until either the parser has
    // read a full field or until there's no tokens left to read
    for (;;) {
      char *maybe_token = top_token();

      // If we're out of tokens to read return whatever's left in the
      // field and row buffers. If there's nothing left, return null.
      if (maybe_token == nullptr) {
        if(m_state == State::END_OF_ROW || m_scanposition == 0) {
          return Field(FieldType::CSV_END);
        }
        m_state = State::END_OF_ROW;
        return Field(std::move(m_fieldbuf));
      }

      // Parsing the CSV is done using a finite state machine
      char c = *maybe_token;
      switch (m_state) {
      case State::START_OF_FIELD:
        m_cursor++;
        if (c == m_terminator) {
          handle_crlf(c);
          m_state = State::END_OF_ROW;
          return Field(std::move(m_fieldbuf));
        }

        if (c == m_quote) {
          m_state = State::IN_QUOTED_FIELD;
        } else if (c == m_delimiter) {
          return Field(std::move(m_fieldbuf));
        } else {
          m_state = State::IN_FIELD;
          m_fieldbuf += c;
        }

        break;

      case State::IN_FIELD:
        m_cursor++;
        if (c == m_terminator) {
          handle_crlf(c);
          m_state = State::END_OF_ROW;
          return Field(std::move(m_fieldbuf));
        }

        if (c == m_delimiter) {
          m_state = State::START_OF_FIELD;
          return Field(std::move(m_fieldbuf));
        }

        m_fieldbuf += c;
        break;

      case State::IN_QUOTED_FIELD:
        m_cursor++;
        if (c == m_quote) {
          m_state = State::IN_ESCAPED_QUOTE;
        } else {
          m_fieldbuf += c;
        }

        break;

      case State::IN_ESCAPED_QUOTE:
        m_cursor++;
        if (c == m_terminator) {
          handle_crlf(c);
          m_state = State::END_OF_ROW;
          return Field(std::move(m_fieldbuf));
        }

        if (c == m_quote) {
          m_state = State::IN_QUOTED_FIELD;
          m_fieldbuf += c;
        } else if (c == m_delimiter) {
          m_state = State::START_OF_FIELD;
          return Field(std::move(m_fieldbuf));
        } else {
          m_state = State::IN_FIELD;
          m_fieldbuf += c;
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
  // When the parser hits the end of a line it needs
  // to check the special case of '\r\n' as a terminator.
  // If it finds that the previous token was a '\r', and
  // the next token will be a '\n', it skips the '\n'.
  void handle_crlf(const char c) {
    if (m_terminator != Term::CRLF || c != '\r') {
      return;
    }

    char *token = top_token();
    if ((token != nullptr) && *token == '\n') {
      m_cursor++;
    }
  }

  // Pulls the next token from the input buffer, but does not move
  // the cursor forward. If the stream is empty and the input buffer
  // is also empty return a nullptr.
  auto top_token() -> char * {
    // Return null if there's nothing left to read
    if (m_eof && m_cursor == m_bytes_read) {
      return nullptr;
    }

    // Refill the input buffer if it's been fully read
    if (m_cursor == m_bytes_read) {
      fill_buffer();
      // Return null if there's nothing left to read
      if (m_bytes_read == 0) {
        return nullptr;
      }
    }

    return &m_inputbuf[m_cursor];
  }

  void fill_buffer() {
    m_input->read(m_inputbuf.data(), INPUTBUF_CAP);
    m_bytes_read = static_cast<size_t>(m_input->gcount());
    m_eof = m_input->eof();
    m_cursor = 0;

    if (m_scanposition == 0 && m_bytes_read >= 3 && m_inputbuf[0] == '\xEF' &&
        m_inputbuf[1] == '\xBB' && m_inputbuf[2] == '\xBF') {
      if (m_bytes_read > 3) {
        m_cursor = 3;
      } else {
        m_bytes_read = 0;
      }
    }

    m_scanposition += static_cast<std::streamoff>(m_bytes_read);
  }

public:
  // Iterator implementation for the CSV parser, which reads
  // from the CSV row by row in the form of a vector of strings
  class iterator {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::vector<std::string>;
    using pointer = const std::vector<std::string> *;
    using reference = const std::vector<std::string> &;
    using iterator_category = std::input_iterator_tag;

    explicit iterator(CsvParser *p, bool end = false) : m_parser(p) {
      static constexpr size_t DEFAULT_ROW_CAPACITY = 50;
      if (!end) {
        m_row.reserve(DEFAULT_ROW_CAPACITY);
        m_current_row = 0;
        next();
      }
    }

    auto operator++() -> iterator & {
      next();
      return *this;
    }

    auto operator++(int) -> iterator {
      iterator i = (*this);
      ++(*this);
      return i;
    }

    auto operator==(const iterator &other) const -> bool {
      return m_current_row == other.m_current_row &&
             m_row.size() == other.m_row.size();
    }

    auto operator!=(const iterator &other) const -> bool {
      return !(*this == other);
    }

    auto operator*() const -> reference { return m_row; }

    auto operator->() const -> pointer { return &m_row; }

  private:
    value_type m_row{};
    CsvParser *m_parser;
    int m_current_row = -1;

    void next() {
      value_type::size_type num_fields = 0;
      for (;;) {
        auto field = m_parser->next_field();
        switch (field.type) {
        case FieldType::CSV_END:
          if (num_fields < m_row.size()) {
            m_row.resize(num_fields);
          }
          m_current_row = -1;
          return;
        case FieldType::ROW_END:
          if (num_fields < m_row.size()) {
            m_row.resize(num_fields);
          }
          m_current_row++;
          return;
        case FieldType::DATA:
          if (num_fields < m_row.size()) {
            m_row[num_fields] = std::move(field.data);
          } else {
            m_row.push_back(std::move(field.data));
          }
          num_fields++;
        }
      }
    }
  };

  auto begin() -> iterator { return iterator(this); };
  auto end() -> iterator { return iterator(this, true); };
};
} // namespace csv
} // namespace aria
#endif
