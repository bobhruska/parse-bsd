# parse-bsd
Parses tcpdump from acurite hub and creates packets to send to weewx

The command line to run it on my pfsense machine:
tcpdump -A -i vtnet1 host 10.1.1.71 and port 80 | ./parse-bsd | xargs -n 1 curl http://10.1.1.106:9999 -s -d > /dev/null 2>&1 &

This sends packets to weewx running interceptor on 10.1.1.106

weewx interceptor:  https://github.com/matthewwall/weewx-interceptor


