########################################################################
### sudo_ttyT_config.sh
### - configure user permissions on Tegra serial ports
###
### Usage (bash):  . sudo_ttyT_config.sh

ls -l /dev/ttyT*
sudo chown :dialout /dev/ttyT*
sudo chmod g+r /dev/ttyT*
ls -l /dev/ttyT*
sudo systemctl stop serial-getty@ttyTCU0.service || true
sudo systemctl is-active serial-getty@ttyTCU0.service || true
