#ifndef NET_INTERNET_SCHEME_H
#define NET_INTERNET_SCHEME_H

// Subset of the schemes.
// http://www.iana.org/assignments/uri-schemes.html

#include "macros/macros.h"

namespace net {
	namespace internet {
		struct scheme {
			public:
				static const size_t kMaxLen = 32;

				static const unsigned char kFile    = 0;
				static const unsigned char kFtp     = 1;
				static const unsigned char kHttp    = 2;
				static const unsigned char kHttps   = 3;
				static const unsigned char kIcap    = 4;
				static const unsigned char kImap    = 5;
				static const unsigned char kLdap    = 6;
				static const unsigned char kMailto  = 7;
				static const unsigned char kNews    = 8;
				static const unsigned char kNfs     = 9;
				static const unsigned char kPop     = 10;
				static const unsigned char kTelnet  = 11;
				static const unsigned char kUnknown = 12;

				unsigned char value;

				// Constructor.
				scheme();
				scheme(unsigned char v);

				const char* name() const;

				bool operator==(unsigned char v) const;
				bool operator!=(unsigned char v) const;

				// Search.
				static scheme search(const char* name, size_t len);

				// Valid scheme character?
				static bool valid_character(unsigned char c);

			private:
				struct _scheme {
					const char* name;
					size_t len;
				};

				static const struct _scheme _M_schemes[];
		};

		inline scheme::scheme()
		{
		}

		inline scheme::scheme(unsigned char v)
		{
			value = v;
		}

		inline const char* scheme::name() const
		{
			if (value >= kUnknown) {
				return NULL;
			}

			return _M_schemes[value].name;
		}

		inline bool scheme::operator==(unsigned char v) const
		{
			return (v == value);
		}

		inline bool scheme::operator!=(unsigned char v) const
		{
			return (v != value);
		}

		inline bool scheme::valid_character(unsigned char c)
		{
			if ((IS_ALPHA(c)) || (IS_DIGIT(c))) {
				return true;
			} else {
				switch (c) {
					case '+':
					case '-':
					case '.':
						return true;
					default:
						return false;
				}
			}
		}
	}
}

#endif // NET_INTERNET_SCHEME_H
