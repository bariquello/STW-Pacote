#!/bin/sh
#############################################################################
# STW-Pacote :: Instalador automatico (bootstrap) - System Way  v0.4
#
# Uso no pfSense. IMPORTANTE: entre no shell "sh" antes (digite: sh) e rode:
#
#   fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
#############################################################################

# ====== CONFIGURACAO (estrutura limpa do repo) =============================
RAW="https://raw.githubusercontent.com/bariquello/STW-Pacote/main"
PKG_URL="${RAW}/stw-pacote-files.tar.gz"
IMG_BASE="${RAW}/files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
# ===========================================================================

VERSION="0.4"
TMP="/tmp/stw_install"
STAGING="/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"

echo "======================================================"
echo " STW-Pacote ${VERSION} - Instalador System Way"
echo "======================================================"

rm -rf "${TMP}"; mkdir -p "${TMP}"

echo "[1/6] Baixando arquivos do pacote..."
fetch -q -o "${TMP}/f.tar.gz" "${PKG_URL}"
[ -s "${TMP}/f.tar.gz" ] || { echo "ERRO: falha ao baixar ${PKG_URL}"; exit 1; }

echo "[2/6] Extraindo arquivos para o sistema..."
tar -xzf "${TMP}/f.tar.gz" -C / 2>/dev/null || { echo "ERRO: extracao falhou."; exit 1; }

echo "[3/6] Baixando logo e favicons..."
mkdir -p "${STAGING}"
for img in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
	fetch -q -o "${STAGING}/${img}" "${IMG_BASE}/${img}"
	[ -s "${STAGING}/${img}" ] && echo "     ok: ${img}" || echo "     AVISO: ${img} nao baixou"
done

echo "[4/6] Ajustando permissoes..."
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null

echo "[5/6] Aplicando tema, menu e gerando backup.sh..."
php -q -r 'require_once("config.inc");require_once("functions.inc");require_once("pkg-utils.inc");require_once("/usr/local/pkg/stw_pacote.inc");if(!is_array($GLOBALS["config"]["installedpackages"]["package"]))$GLOBALS["config"]["installedpackages"]["package"]=array();$f=false;foreach($GLOBALS["config"]["installedpackages"]["package"] as $p){if(isset($p["name"])&&$p["name"]=="STW-Pacote")$f=true;}if(!$f){$GLOBALS["config"]["installedpackages"]["package"][]=array("name"=>"STW-Pacote","internal_name"=>"STW-Pacote","descr"=>"System Way: tema + Backup FTP","version"=>"0.4","configurationfile"=>"stw_pacote.xml");}stw_pacote_install();echo "instalado\n";'

echo "[6/6] Verificando /etc/backup.sh..."
[ -s /etc/backup.sh ] && echo "     backup.sh OK ($(wc -c < /etc/backup.sh) bytes)" || echo "     (configure em Services > STW Backup FTP e salve)"

rm -rf "${TMP}"
echo ""
echo "======================================================"
echo " Concluido! Recarregue a web com Ctrl+F5 (ou relogin)."
echo " Menu: Services > STW Backup FTP"
echo "======================================================"
exit 0
