#!/bin/sh
# rc script minimo para o servico STW Backup (executa a rotina sob demanda)
case "$1" in
    start)  [ -x /etc/backup.sh ] && /bin/sh /etc/backup.sh ;;
    stop)   ;;
    status) [ -f /etc/backup.sh ] && echo "STW Backup FTP configurado" || echo "STW Backup FTP nao configurado" ;;
    *)      echo "Uso: $0 {start|stop|status}" ;;
esac
exit 0
