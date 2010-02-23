#!/bin/sh
exec >$1 2>&1
exec dpkg-query --showformat='${Package} ${Version} ${Status}\n' -W

