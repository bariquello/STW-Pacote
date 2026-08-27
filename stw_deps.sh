#!/bin/sh
# STW-Pacote :: instala APENAS as dependencias (Cron + Zabbix 5.0 + script zbx)
# Idempotente: nao reinstala o que ja existe.
echo "===== STW :: Dependencias ====="
for p in pfSense-pkg-Cron pfSense-pkg-zabbix-agent5 pfSense-pkg-zabbix-proxy5; do
  if pkg-static info -e "$p" >/dev/null 2>&1; then
    echo "[=] $p ja instalado"
  else
    echo "[+] instalando $p ..."
    pkg-static install -y "$p" && echo "    ok" || echo "    FALHA"
  fi
done
if [ -s /root/scripts/pfsense_zbx.php ]; then
  echo "[=] pfsense_zbx.php ja existe"
else
  echo "[+] baixando pfsense_zbx.php ..."
  mkdir -p /root/scripts
  fetch -q -o /root/scripts/pfsense_zbx.php https://raw.githubusercontent.com/rbicelli/pfsense-zabbix-template/master/pfsense_zbx.php \
    || curl -sSL -o /root/scripts/pfsense_zbx.php https://raw.githubusercontent.com/rbicelli/pfsense-zabbix-template/master/pfsense_zbx.php
  [ -s /root/scripts/pfsense_zbx.php ] && chmod 755 /root/scripts/pfsense_zbx.php && echo "    ok" || echo "    FALHA"
fi
echo "===== Concluido ====="
