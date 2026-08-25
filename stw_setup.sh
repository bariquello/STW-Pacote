#!/bin/sh
# STW-Pacote v0.5 - Instalador (rode dentro do shell "sh")
RAW="https://raw.githubusercontent.com/bariquello/STW-Pacote/main"
TMP="/tmp/stw_install"
W="/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
echo "===== STW-Pacote 0.5 - System Way ====="
rm -rf "$TMP"; mkdir -p "$TMP"
echo "[1/5] Baixando pacote..."
fetch -q -o "$TMP/f.tar.gz" "$RAW/stw-pacote-files.tar.gz"
[ -s "$TMP/f.tar.gz" ] || { echo "ERRO: download falhou"; exit 1; }
echo "[2/5] Extraindo..."
tar -xzf "$TMP/f.tar.gz" -C /
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null
echo "[3/5] Garantindo logo/favicons..."
mkdir -p "$W"
for img in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
  [ -s "$W/$img" ] || fetch -q -o "$W/$img" "$RAW/files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www/$img"
  [ -s "$W/$img" ] && echo "   ok: $img" || echo "   AVISO: $img ausente"
done
echo "[4/5] Registrando pacote, tema, menu e gerando backup.sh..."
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");$c=&$GLOBALS["config"];if(!is_array($c["installedpackages"]["package"]))$c["installedpackages"]["package"]=array();$f=false;foreach($c["installedpackages"]["package"] as $p){if(isset($p["name"])&&$p["name"]=="STW-Pacote")$f=true;}if(!$f){$c["installedpackages"]["package"][]=array("name"=>"STW-Pacote","internal_name"=>"STW-Pacote","descr"=>"System Way: tema + Backup FTP","version"=>"0.5","configurationfile"=>"stw_pacote.xml");}stw_pacote_install();echo "instalado\n";'
echo "[5/5] Verificando..."
[ -s /etc/backup.sh ] && echo "   backup.sh OK ($(wc -c < /etc/backup.sh) bytes)" || echo "   pendente"
rm -rf "$TMP"
echo "===== Concluido. Ctrl+F5 na web. Menu: Services > STW Backup FTP ====="
