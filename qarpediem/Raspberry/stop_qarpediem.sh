#! /bin/sh

gksudo -u root systemctl stop qarpediem_datasender.service
gksudo -u root systemctl stop qarpediem_datapoller.service
