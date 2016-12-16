salt -G "os:redhat" cmd.run 'echo "live" > /tmp/htrd_minion_alived'
salt -G "os:CentOS7" cmd.run 'echo "live" > /tmp/htrd_minion_alived'
salt -G "os:windows" cmd.run 'echo "live" > %temp%\htrd_minion_alived'
