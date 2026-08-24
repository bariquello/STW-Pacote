#!/bin/sh
#############################################################################
# STW-Pacote :: Instalador automatico (bootstrap) - System Way
# Uso no pfSense (Shell opcao 8 OU Diagnostics > Command Prompt):
#
#   fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
#############################################################################

# ====== CONFIGURACAO =======================================================
# Tarball com a logica + tema (css/js/head.inc/xml) - arvore relativa a /
PKG_URL="https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw-pacote-files.tar.gz"

# Base RAW onde estao os binarios do tema (logo/favicons) no seu GitHub.
# (caminho profundo conforme voce subiu os arquivos)
IMG_BASE="https://raw.githubusercontent.com/bariquello/STW-Pacote/main/pfSense-pkg-STW-Pacote/pfSense-pkg-STW-Pacote/sysutils/pfSense-pkg-STW-Pacote/files/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"
# ===========================================================================

VERSION="0.3"
TMP="/tmp/stw_install"
TARBALL="${TMP}/stw-pacote-files.tar.gz"
STAGING="/usr/local/share/pfSense-pkg-STW-Pacote/theme/www"

echo "======================================================"
echo " STW-Pacote ${VERSION} - Instalador System Way"
echo "======================================================"

# 1) Preparacao ------------------------------------------------------------
rm -rf "${TMP}"
mkdir -p "${TMP}"

echo "[1/6] Baixando arquivos do pacote..."
fetch -q -o "${TARBALL}" "${PKG_URL}"
if [ ! -s "${TARBALL}" ]; then
	echo "ERRO: falha ao baixar ${PKG_URL}"
	exit 1
fi

# 2) Extracao nos caminhos corretos ---------------------------------------
echo "[2/6] Extraindo arquivos para o sistema..."
tar -xzf "${TARBALL}" -C / 2>/dev/null || { echo "ERRO: falha ao extrair."; exit 1; }

# 3) Baixando binarios do tema (logo/favicons) direto do GitHub ------------
echo "[3/6] Baixando logo e favicons..."
mkdir -p "${STAGING}"
for img in logo.png logo-branco.png favicon.ico favicon-16x16.png favicon-32x32.png; do
	fetch -q -o "${STAGING}/${img}" "${IMG_BASE}/${img}"
	if [ -s "${STAGING}/${img}" ]; then
		echo "     ok: ${img}"
	else
		echo "     AVISO: nao baixou ${img} (verifique o caminho no GitHub)"
	fi
done

# 4) Permissoes ------------------------------------------------------------
echo "[4/6] Ajustando permissoes..."
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null
[ -f /etc/backup.sh ] && chmod 777 /etc/backup.sh 2>/dev/null

# 5) Registro + sync do pacote (menu, servico, deploy do tema) -------------
echo "[5/6] Registrando o pacote e sincronizando (menu + tema)..."
/usr/local/bin/php -q <<'PHP'
<?php
require_once("config.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");

global $config;

if (!is_array($config['installedpackages'])) {
	$config['installedpackages'] = array();
}
if (!is_array($config['installedpackages']['package'])) {
	$config['installedpackages']['package'] = array();
}

// Registra a entrada do pacote (se ainda nao existir)
$found = false;
foreach ($config['installedpackages']['package'] as $p) {
	if (isset($p['name']) && $p['name'] == 'STW-Pacote') { $found = true; break; }
}
if (!$found) {
	$config['installedpackages']['package'][] = array(
		'name'              => 'STW-Pacote',
		'internal_name'     => 'STW-Pacote',
		'descr'             => 'System Way: tema pfSense-Systemway + Backup FTP',
		'version'           => '0.3',
		'configurationfile' => 'stw_pacote.xml'
	);
	echo "  -> Pacote registrado no config.xml\n";
} else {
	echo "  -> Pacote ja registrado\n";
}
write_config("STW-Pacote: registro do pacote (instalacao via script)");

// sync_package le o XML e instala MENU + SERVICO + roda o install command
if (function_exists('sync_package')) {
	sync_package("STW-Pacote");
	echo "  -> sync_package executado (menu Services + tema aplicados)\n";
} else {
	// fallback: aplica ao menos o tema
	require_once("/usr/local/pkg/stw_pacote.inc");
	if (function_exists('stw_pacote_install')) { stw_pacote_install(); }
	echo "  -> AVISO: sync_package indisponivel; tema aplicado via fallback\n";
}
PHP

# 6) Finalizacao -----------------------------------------------------------
echo "[6/6] Limpando temporarios..."
rm -rf "${TMP}"

echo ""
echo "======================================================"
echo " Instalacao concluida!"
echo "------------------------------------------------------"
echo " - Tema + logo aplicados (recarregue o navegador com Ctrl+F5)"
echo " - Menu: Services > STW Backup FTP"
echo "======================================================"
exit 0
