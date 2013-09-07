#include <stdlib.h>
#include <netinet/in.h>
#include "net/internet/url.h"

net::internet::url::url() : _M_scheme(scheme::kUnknown)
{
	_M_tokenlen = 0;
	_M_userinfolen = 0;
	_M_hostlen = 0;
	_M_port = 0;
	_M_pathlen = 0;
	_M_querylen = 0;
	_M_fragmentlen = 0;

	_M_state = 0;
}

void net::internet::url::reset()
{
	_M_buf.reset();

	_M_scheme.value = scheme::kUnknown;

	_M_tokenlen = 0;
	_M_userinfolen = 0;
	_M_hostlen = 0;
	_M_port = 0;
	_M_pathlen = 0;
	_M_querylen = 0;
	_M_fragmentlen = 0;

	_M_state = 0;
}

net::internet::url::parse_result net::internet::url::parse(const void* buf, size_t count)
{
	const unsigned char* begin = reinterpret_cast<const unsigned char*>(buf);
	const unsigned char* end = begin + count;
	const unsigned char* ptr = begin;

	while (ptr < end) {
		unsigned char c = *ptr;

		switch (_M_state) {
			case 0: // Initial state.
				if (scheme::valid_character(c)) {
					_M_token = ptr - begin;
					_M_tokenlen = 1;

					_M_state = 1; // Scheme.
				} else {
					switch (c) {
						case '/':
							_M_path = ptr - begin;
							_M_pathlen = 1;

							_M_state = 15; // Path.
							break;
						case ' ':
						case '\t':
						case '\r':
						case '\n':
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 1: // Scheme.
				if (scheme::valid_character(c)) {
					if (++_M_tokenlen > scheme::kMaxLen) {
						return kInvalidUrl;
					}
				} else if (c == ':') {
					if ((_M_scheme = scheme::search(reinterpret_cast<const char*>(begin) + _M_token, _M_tokenlen)) == scheme::kUnknown) {
						return kInvalidUrl;
					}

					_M_state = 2; // After scheme:
				} else {
					return kInvalidUrl;
				}

				break;
			case 2: // After scheme:
				if (c == '/') {
					_M_state = 3; // After scheme:/
				} else {
					return kInvalidUrl;
				}

				break;
			case 3: // After scheme:/
				if (c == '/') {
					_M_state = 4; // After scheme://
				} else {
					return kInvalidUrl;
				}

				break;
			case 4: // After scheme://
				if (is_unreserved(c)) {
					_M_token = ptr - begin;

					_M_ip_literal = 0;

					_M_state = 8; // Authority.
				} else if (c == '[') {
					_M_ip_literal = 1;

					_M_state = 5; // Before IP-literal.
				} else {
					return kInvalidUrl;
				}

				break;
			case 5: // Before IP-literal.
				if (IS_XDIGIT(c)) {
					_M_host = ptr - begin;
					_M_hostlen = 1;

					_M_state = 6; // IP-literal.
				} else {
					switch (c) {
						case ':':
						case '.':
							_M_host = ptr - begin;
							_M_hostlen = 1;

							_M_state = 6; // IP-literal.
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 6: // IP-literal.
				if (IS_XDIGIT(c)) {
					if (++_M_hostlen > INET6_ADDRSTRLEN) {
						return kInvalidUrl;
					}
				} else {
					switch (c) {
						case ':':
						case '.':
							if (++_M_hostlen > INET6_ADDRSTRLEN) {
								return kInvalidUrl;
							}

							break;
						case ']':
							_M_state = 7; // After IP-literal.
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 7: // After IP-literal.
				switch (c) {
					case ':': // Before port.
						_M_state = 13; // Before port.
						break;
					case '/':
						_M_path = ptr - begin;
						_M_pathlen = 1;

						_M_state = 15; // Path.
						break;
					default:
						return kInvalidUrl;
				}

				break;
			case 8: // Authority.
				if (!is_unreserved(c)) {
					switch (c) {
						case ':':
							_M_host = _M_token;
							_M_hostlen = (ptr - begin) - _M_token;

							_M_state = 9; // Before password or port.
							break;
						case '@':
							if ((_M_userinfolen = (ptr - begin) - _M_token) > kUserinfoMaxLen) {
								return kUrlTooLong;
							}

							_M_userinfo = _M_token;

							_M_state = 11; // Before host.
							break;
						case '/':
							if (_M_userinfolen > 0) {
								return kInvalidUrl;
							}

							_M_path = ptr - begin;

							if ((_M_hostlen = _M_path - _M_token) > kHostMaxLen) {
								return kUrlTooLong;
							}

							_M_host = _M_token;

							_M_pathlen = 1;

							_M_state = 15; // Path.
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 9: // Before password or port.
				if ((_M_userinfolen == 0) && (IS_DIGIT(c))) {
					_M_port = c - '0';

					_M_state = 10; // Password or port.
				} else {
					if (is_unreserved(c)) {
						_M_userinfo = _M_token;
						_M_userinfolen = (ptr - begin) - _M_token;

						_M_state = 8; // Authority.
					} else {
						return kInvalidUrl;
					}
				}

				break;
			case 10: // Password or port.
				if (IS_DIGIT(c)) {
					unsigned port;
					if ((port = (_M_port * 10) + (c - '0')) > 65535) {
						_M_userinfo = _M_token;
						_M_userinfolen = (ptr - begin) - _M_token;

						_M_state = 8; // Authority.
					} else {
						_M_port = port;
					}
				} else {
					if (is_unreserved(c)) {
						_M_userinfo = _M_token;
						_M_userinfolen = (ptr - begin) - _M_token;

						_M_state = 8; // Authority.
					} else if (c == '/') {
						if (_M_hostlen > kHostMaxLen) {
							return kUrlTooLong;
						}

						if (_M_port == 0) {
							return kInvalidUrl;
						}

						_M_path = ptr - begin;
						_M_pathlen = 1;

						_M_state = 15; // Path.
					} else {
						return kInvalidUrl;
					}
				}

				break;
			case 11: // Before host.
				if (is_unreserved(c)) {
					_M_host = ptr - begin;
					_M_hostlen = 1;

					_M_state = 12; // Host.
				} else {
					return kInvalidUrl;
				}

				break;
			case 12: // Host.
				if (is_unreserved(c)) {
					if (++_M_hostlen > kHostMaxLen) {
						return kUrlTooLong;
					}
				} else {
					switch (c) {
						case ':':
							_M_state = 13; // Before port.
							break;
						case '/':
							_M_path = ptr - begin;
							_M_pathlen = 1;

							_M_state = 15; // Path.
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 13: // Before port.
				if (IS_DIGIT(c)) {
					_M_port = c - '0';

					_M_state = 14; // Port.
				} else {
					return kInvalidUrl;
				}

				break;
			case 14: // Port.
				if (IS_DIGIT(c)) {
					unsigned port;
					if ((port = (_M_port * 10) + (c - '0')) > 65535) {
						return kInvalidUrl;
					}

					_M_port = port;
				} else if (c == '/') {
					if (_M_port == 0) {
						return kInvalidUrl;
					}

					_M_path = ptr - begin;
					_M_pathlen = 1;

					_M_state = 15; // Path.
				} else {
					return kInvalidUrl;
				}

				break;
			case 15: // Path.
				if (is_pchar(c)) {
					if (++_M_pathlen > kPathMaxLen) {
						return kUrlTooLong;
					}
				} else {
					switch (c) {
						case '/':
							if (++_M_pathlen > kPathMaxLen) {
								return kUrlTooLong;
							}

							break;
						case '?':
							_M_query = ptr - begin;
							_M_querylen = 1;

							_M_state = 16; // Query;
							break;
						case '#':
							_M_fragment = ptr - begin;
							_M_fragmentlen = 1;

							_M_state = 17; // Fragment;
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 16: // Query.
				if (is_pchar(c)) {
					if (++_M_querylen > kQueryMaxLen) {
						return kUrlTooLong;
					}
				} else {
					switch (c) {
						case '/':
						case '?':
							if (++_M_querylen > kQueryMaxLen) {
								return kUrlTooLong;
							}

							break;
						case '#':
							_M_fragment = ptr - begin;
							_M_fragmentlen = 1;

							_M_state = 17; // Fragment;
							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
			case 17: // Fragment.
				if (is_pchar(c)) {
					if (++_M_fragmentlen > kFragmentMaxLen) {
						return kUrlTooLong;
					}
				} else {
					switch (c) {
						case '/':
						case '?':
							if (++_M_fragmentlen > kFragmentMaxLen) {
								return kUrlTooLong;
							}

							break;
						default:
							return kInvalidUrl;
					}
				}

				break;
		}

		ptr++;
	}

	switch (_M_state) {
		case 8: // Authority.
			if (_M_userinfolen > 0) {
				return kInvalidUrl;
			}

			if ((_M_hostlen = (ptr - begin) - _M_token) > kHostMaxLen) {
				return kUrlTooLong;
			}

			_M_host = _M_token;
			break;
		case 10: // Password or port.
			if (_M_hostlen > kHostMaxLen) {
				return kUrlTooLong;
			}

			if (_M_port == 0) {
				return kInvalidUrl;
			}

			break;
		case 14: // Port.
			if (_M_port == 0) {
				return kInvalidUrl;
			}

			break;
		case 7: // After IP-literal.
		case 12: // Host.
		case 15: // Path.
		case 16: // Query.
		case 17: // Fragment.
			break;
		default:
			return kInvalidUrl;
	}

	if (_M_userinfolen > 0) {
		// Save userinfo.
		size_t count = _M_buf.count();

		if (!_M_buf.append_nul_terminated_string(reinterpret_cast<const char*>(begin) + _M_userinfo, _M_userinfolen)) {
			return kNoMemory;
		}

		_M_userinfo = count;
	}

	if (_M_hostlen > 0) {
		// Save host.
		size_t count = _M_buf.count();

		if (!_M_buf.append_nul_terminated_string(reinterpret_cast<const char*>(begin) + _M_host, _M_hostlen)) {
			return kNoMemory;
		}

		_M_host = count;
	}

	if (_M_pathlen > 0) {
		// Save path.
		size_t count = _M_buf.count();

		if (!_M_buf.append_nul_terminated_string(reinterpret_cast<const char*>(begin) + _M_path, _M_pathlen)) {
			return kNoMemory;
		}

		_M_path = count;
	}

	if (_M_querylen > 0) {
		// Save query.
		size_t count = _M_buf.count();

		if (!_M_buf.append_nul_terminated_string(reinterpret_cast<const char*>(begin) + _M_query, _M_querylen)) {
			return kNoMemory;
		}

		_M_query = count;
	}

	if (_M_fragmentlen > 0) {
		// Save fragment.
		size_t count = _M_buf.count();

		if (!_M_buf.append_nul_terminated_string(reinterpret_cast<const char*>(begin) + _M_fragment, _M_fragmentlen)) {
			return kNoMemory;
		}

		_M_fragment = count;
	}

	return kSuccess;
}

bool net::internet::url::encode(const char* url, size_t len, string::buffer& buf)
{
	if (!buf.allocate(len * 3)) {
		return false;
	}

	const char* end = url + len;

	char* d = buf.data() + buf.count();
	char* dest = d;

	while (url < end) {
		unsigned char c = (unsigned char) *url++;

		if ((is_unreserved(c)) || (c == '/')) {
			*dest++ = c;
		} else {
			*dest++ = '%';

			unsigned char n;
			if ((n = c / 16) < 10) {
				*dest++ = '0' + n;
			} else {
				*dest++ = 'a' + (n - 10);
			}

			if ((n = c % 16) < 10) {
				*dest++ = '0' + n;
			} else {
				*dest++ = 'a' + (n - 10);
			}
		}
	}

	buf.increment_count(dest - d);

	return true;
}
