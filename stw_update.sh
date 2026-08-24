#!/bin/sh
# STW-Pacote :: Atualizacao v0.4  (rode dentro do shell "sh")
RAW="https://raw.githubusercontent.com/bariquello/STW-Pacote/main"
TMP="/tmp/stw_upd"
echo "===== STW-Pacote :: Atualizacao v0.4 ====="
rm -rf "$TMP"; mkdir -p "$TMP"
echo "[1/3] Baixando pacote..."
fetch -q -o "$TMP/f.tar.gz" "$RAW/stw-pacote-files.tar.gz"
[ -s "$TMP/f.tar.gz" ] || { echo "ERRO: download falhou"; exit 1; }
tar -xzf "$TMP/f.tar.gz" -C /
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null
echo "[2/3] Baixando logo/favicons..."
W="/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
for img in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
  fetch -q -o "$W/$img" "$RAW/files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/$img"
done
echo "[3/3] Aplicando e gerando backup.sh..."
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");stw_deploy_theme();stw_aplicar_tema();stw_registrar_menu();stw_pacote_resync();echo file_exists("/etc/backup.sh")?"backup.sh OK\n":"pendente\n";'
rm -rf "$TMP"
echo "===== Concluido. Ctrl+F5 na web. ====="
