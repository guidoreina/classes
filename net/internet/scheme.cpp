#include <stdlib.h>
#include <string.h>
#include "net/internet/scheme.h"

const struct net::internet::scheme::_scheme net::internet::scheme::_M_schemes[] = {
	{"file", 4},
	{"ftp", 3},
	{"http", 4},
	{"https", 5},
	{"icap", 4},
	{"imap", 4},
	{"ldap", 4},
	{"mailto", 6},
	{"news", 4},
	{"nfs", 3},
	{"pop", 3},
	{"telnet", 6}
};

net::internet::scheme net::internet::scheme::search(const char* name, size_t len)
{
	int i = 0;
	int j = ARRAY_SIZE(_M_schemes) - 1;

	while (i <= j) {
		int pivot = (i + j) / 2;
		int ret = strncasecmp(name, _M_schemes[pivot].name, len);
		if (ret < 0) {
			j = pivot - 1;
		} else if (ret == 0) {
			if (len < _M_schemes[pivot].len) {
				j = pivot - 1;
			} else if (len == _M_schemes[pivot].len) {
				return scheme(pivot);
			} else {
				i = pivot + 1;
			}
		} else {
			i = pivot + 1;
		}
	}

	return scheme(kUnknown);
}
