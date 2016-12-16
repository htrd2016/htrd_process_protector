salt -G "os:redhat" cmd.run_all '/root/minion_protector/htrdMinionProtector-el5 /tmp/htrd_minion_alived "service salt-minion stop" "service salt-minion start" 300 >/dev/null 2>log &'

#salt -G "os:CentOS7" cmd.run_all '/root/minion_protector/htrdMinionProtector-el7 /tmp/htrd_minion_alived "systemctl salt-minion stop" "systemctl start salt-minion" 300'

salt -G "os:windows" cmd.run_all 'start C:\\minion_protector\\htrdMinionProtector.exe %temp%\\htrd_minion_alived "net stop salt-minion" "net start salt-minion" 300'
