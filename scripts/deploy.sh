#!/bin/bash
apt-get update && apt-get install -y xinetd python3
cp blpfsm.xinetd /etc/xinetd.d/blpfsm

rm -rf /home/cykor/*
cp -r *.sh /home/cykor/
cp -r backend /home/cykor/
cp -r frontend /home/cykor/
chown 1000:1000 *.sh
chown -R 1000:1000 /home/cykor/backend
chown -R 1000:1000 /home/cykor/frontend

su cykor -c "crontab -l | grep /home/cykor/cleaner.sh || (crontab -l 2>/dev/null; echo \"*/5 * * * * /home/cykor/cleaner.sh\") | crontab -"

/etc/init.d/xinetd stop || true
/etc/init.d/xinetd start