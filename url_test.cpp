#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "net/internet/url.h"

int main()
{
	net::internet::url url;

	printf("Enter URL: ");

	char line[4 * 1024];
	while (fgets(line, sizeof(line), stdin)) {
		size_t len;
		if ((len = strlen(line)) <= 1) {
			printf("Enter URL: ");
			continue;
		}

		if ((len == 5) && (strncasecmp(line, "quit", 4) == 0)) {
			break;
		}

		if (line[len - 1] == '\n') {
			len--;
		}

		if (len > 0) {
			url.reset();
			switch (url.parse(line, len)) {
				case net::internet::url::kNoMemory:
					printf("No memory.\n");
					return -1;
				case net::internet::url::kInvalidUrl:
					printf("Invalid URL.\n");
					break;
				case net::internet::url::kUrlTooLong:
					printf("URL too long.\n");
					break;
				default:
					printf("\tScheme = [%s].\n", url.scheme().name());
					printf("\tUserinfo = [%s].\n", url.userinfo());
					printf("\tIP literal? %s.\n", url.ip_literal() ? "yes" : "no");
					printf("\tHost = [%s].\n", url.host());
					printf("\tPort = [%u].\n", url.port());
					printf("\tPath = [%s].\n", url.path());
					printf("\tQuery = [%s].\n", url.query());
					printf("\tFragment = [%s].\n", url.fragment());
			}
		}

		printf("Enter URL: ");
	}

	return 0;
}
