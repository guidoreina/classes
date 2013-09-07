#ifndef NET_INTERNET_URL_H
#define NET_INTERNET_URL_H

#include "net/internet/scheme.h"
#include "string/buffer.h"
#include "macros/macros.h"

namespace net {
	namespace internet {
		bool is_unreserved(unsigned char c);
		bool is_reserved(unsigned char c);
		bool is_gen_delim(unsigned char c);
		bool is_sub_delim(unsigned char c);
		bool is_pchar(unsigned char c);

		class url {
			public:
				static const size_t kUserinfoMaxLen = 255;
				static const size_t kHostMaxLen = 255;
				static const size_t kPathMaxLen = 16 * 1024;
				static const size_t kQueryMaxLen = 16 * 1024;
				static const size_t kFragmentMaxLen = 16 * 1024;

				// Constructor.
				url();

				// Destructor.
				~url();

				// Reset.
				void reset();

				// Get scheme.
				const struct scheme& scheme() const;

				// Get userinfo.
				const char* userinfo() const;
				const char* userinfo(unsigned short& len) const;

				// Get host.
				const char* host() const;
				const char* host(unsigned short& len) const;

				// IP literal?
				bool ip_literal() const;

				// Get port.
				unsigned short port() const;

				// Get path.
				const char* path() const;
				const char* path(unsigned short& len) const;

				// Get query.
				const char* query() const;
				const char* query(unsigned short& len) const;

				// Get fragment.
				const char* fragment() const;
				const char* fragment(unsigned short& len) const;

				// Parse.
				enum parse_result {
					kNoMemory,
					kInvalidUrl,
					kUrlTooLong,
					kSuccess
				};

				parse_result parse(const void* buf, size_t count);

				// Encode.
				static bool encode(const char* url, size_t len, string::buffer& buf);

			private:
				string::buffer _M_buf;

				struct scheme _M_scheme;

				unsigned short _M_token;
				unsigned short _M_tokenlen;
				unsigned short _M_userinfo;
				unsigned short _M_userinfolen;
				unsigned short _M_host;
				unsigned short _M_hostlen;
				unsigned short _M_port;
				unsigned short _M_path;
				unsigned short _M_pathlen;
				unsigned short _M_query;
				unsigned short _M_querylen;
				unsigned short _M_fragment;
				unsigned short _M_fragmentlen;

				unsigned char _M_ip_literal:1;

				unsigned char _M_state:6;
		};

		inline bool is_unreserved(unsigned char c)
		{
			if ((IS_ALPHA(c)) || (IS_DIGIT(c))) {
				return true;
			} else {
				switch (c) {
					case '-':
					case '.':
					case '_':
					case '~':
						return true;
					default:
						return false;
				}
			}
		}

		inline bool is_reserved(unsigned char c)
		{
			return ((is_gen_delim(c)) || (is_sub_delim(c)));
		}

		inline bool is_gen_delim(unsigned char c)
		{
			switch (c) {
				case ':':
				case '/':
				case '?':
				case '#':
				case '[':
				case ']':
				case '@':
					return true;
				default:
					return false;
			}
		}

		inline bool is_sub_delim(unsigned char c)
		{
			switch (c) {
				case '!':
				case '$':
				case '&':
				case '\'':
				case '(':
				case ')':
				case '*':
				case '+':
				case ',':
				case ';':
				case '=':
					return true;
				default:
					return false;
			}
		}

		inline bool is_pchar(unsigned char c)
		{
			if ((is_unreserved(c)) || (is_sub_delim(c))) {
				return true;
			} else {
				switch (c) {
					case '%':
					case ':':
					case '@':
						return true;
					default:
						return false;
				}
			}
		}

		inline url::~url()
		{
		}

		inline const struct scheme& url::scheme() const
		{
			return _M_scheme;
		}

		inline const char* url::userinfo() const
		{
			return (_M_userinfolen == 0) ? NULL : _M_buf.data() + _M_userinfo;
		}

		inline const char* url::userinfo(unsigned short& len) const
		{
			len = _M_userinfolen;
			return (_M_userinfolen == 0) ? NULL : _M_buf.data() + _M_userinfo;
		}

		inline const char* url::host() const
		{
			return (_M_hostlen == 0) ? NULL : _M_buf.data() + _M_host;
		}

		inline const char* url::host(unsigned short& len) const
		{
			len = _M_hostlen;
			return (_M_hostlen == 0) ? NULL : _M_buf.data() + _M_host;
		}

		inline bool url::ip_literal() const
		{
			return (bool) _M_ip_literal;
		}

		inline unsigned short url::port() const
		{
			return _M_port;
		}

		inline const char* url::path() const
		{
			return (_M_pathlen == 0) ? "/" : _M_buf.data() + _M_path;
		}

		inline const char* url::path(unsigned short& len) const
		{
			if (_M_pathlen == 0) {
				len = 1;
				return "/";
			} else {
				len = _M_pathlen;
				return _M_buf.data() + _M_path;
			}
		}

		inline const char* url::query() const
		{
			return (_M_querylen == 0) ? NULL : _M_buf.data() + _M_query;
		}

		inline const char* url::query(unsigned short& len) const
		{
			len = _M_querylen;
			return (_M_querylen == 0) ? NULL : _M_buf.data() + _M_query;
		}

		inline const char* url::fragment() const
		{
			return (_M_fragmentlen == 0) ? NULL : _M_buf.data() + _M_fragment;
		}

		inline const char* url::fragment(unsigned short& len) const
		{
			len = _M_fragmentlen;
			return (_M_fragmentlen == 0) ? NULL : _M_buf.data() + _M_fragment;
		}
	}
}

#endif // NET_INTERNET_URL_H
