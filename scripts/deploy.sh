#!/bin/bash
apt-get update && apt-get install -y xinetd python3
cp blpfsm.xinetd /etc/xinetd.d/blpfsm
useradd -m -d /home/user -s /bin/bash -u 1001 user 2>/dev/null || true

rm -rf /home/user/*
cp -r *.sh /home/user/
cp -r backend /home/user/
cp -r frontend /home/user/
chown 1001:1001 *.sh
chown -R 1001:1001 /home/user/backend
chown -R 1001:1001 /home/user/frontend

su user -c "crontab -l | grep /home/user/cleaner.sh || (crontab -l 2>/dev/null; echo \"*/5 * * * * /home/user/cleaner.sh\") | crontab -"

/etc/init.d/xinetd stop || true
/etc/init.d/xinetd start