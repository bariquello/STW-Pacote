#!/bin/sh
# STW-Pacote v0.6 - Atualizacao (rode dentro do shell "sh")
RAW="https://raw.githubusercontent.com/bariquello/STW-Pacote/main"
TMP="/tmp/stw_upd"
echo "===== STW-Pacote :: Atualizacao 0.6 ====="
rm -rf "$TMP"; mkdir -p "$TMP"
fetch -q -o "$TMP/f.tar.gz" "$RAW/stw-pacote-files.tar.gz"
[ -s "$TMP/f.tar.gz" ] || { echo "ERRO: download falhou"; exit 1; }
tar -xzf "$TMP/f.tar.gz" -C /
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");foreach(stw_instalar_dependencias() as $l)echo "  $l\n";echo "  ".stw_instalar_zbx_script(false)."\n";stw_deploy_theme();stw_aplicar_tema();stw_registrar_menu();stw_pacote_resync();echo "  atualizado\n";'
grep -E "^HOST=|^USUARIO=|^SENHA=" /etc/backup.sh
rm -rf "$TMP"
echo "===== Concluido. Ctrl+F5 na web. ====="
