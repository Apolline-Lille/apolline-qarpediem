#! /bin/bash

gksudo -u root systemctl start qarpediem_datasender.service
gksudo -u root systemctl start qarpediem_datapoller.service

#-----------Alpha---------------------
#sudo -u root chgrp -R qarpediem *db*
#sudo -u root chown -R qarpediem *db*
