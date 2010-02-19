#!/bin/sh
exec >$1 2>&1
cat <<EOF
<h1>Installed packages</h1><br>
EOF
dpkg-query --showformat='<left>${Package}</left> <right>${Version}</right>\n' -W
echo

