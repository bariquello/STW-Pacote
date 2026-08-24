#!/bin/sh
#############################################################################
# STW-Pacote :: Instalador automatico (bootstrap) - System Way
# Uso no pfSense (Shell opcao 8 OU Diagnostics > Command Prompt):
#
#   fetch -q -o - "https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw_setup.sh" | sh
#
# O script baixa os arquivos do tema + logica do STW-Pacote, instala nos
# caminhos corretos, registra o pacote no config.xml (menu Services aparece)
# e executa o hook de instalacao (aplica tema + prepara backup FTP).
#############################################################################

# ====== CONFIGURACAO (ajuste apenas se mudar de repo/branch) ===============
# URL do tarball com os arquivos do pacote (arvore relativa a /).
PKG_URL="https://raw.githubusercontent.com/bariquello/STW-Pacote/main/stw-pacote-files.tar.gz"
# ===========================================================================

VERSION="0.3"
TMP="/tmp/stw_install"
TARBALL="${TMP}/stw-pacote-files.tar.gz"

echo "======================================================"
echo " STW-Pacote ${VERSION} - Instalador System Way"
echo "======================================================"

# 1) Preparacao ------------------------------------------------------------
rm -rf "${TMP}"
mkdir -p "${TMP}"

echo "[1/5] Baixando arquivos do pacote..."
fetch -q -o "${TARBALL}" "${PKG_URL}"
if [ ! -s "${TARBALL}" ]; then
	echo "ERRO: falha ao baixar ${PKG_URL}"
	echo "Verifique a URL e a conectividade do firewall."
	exit 1
fi

# 2) Extracao nos caminhos corretos ---------------------------------------
echo "[2/5] Extraindo arquivos para o sistema..."
tar -xzf "${TARBALL}" -C / 2>/dev/null
if [ $? -ne 0 ]; then
	echo "ERRO: falha ao extrair o tarball."
	exit 1
fi

# 3) Permissoes ------------------------------------------------------------
echo "[3/5] Ajustando permissoes..."
chmod +x /usr/local/etc/rc.d/stw_backup.sh 2>/dev/null
[ -f /etc/backup.sh ] && chmod 777 /etc/backup.sh 2>/dev/null

# 4) Registro do pacote no config.xml + hook de instalacao -----------------
echo "[4/5] Registrando o pacote e aplicando o tema..."
/usr/local/bin/php -q <<'PHP'
<?php
require_once("config.inc");
require_once("functions.inc");
require_once("pkg-utils.inc");
require_once("/usr/local/pkg/stw_pacote.inc");

global $config;

if (!is_array($config['installedpackages'])) {
	$config['installedpackages'] = array();
}
if (!is_array($config['installedpackages']['package'])) {
	$config['installedpackages']['package'] = array();
}

$found = false;
foreach ($config['installedpackages']['package'] as $p) {
	if (isset($p['name']) && $p['name'] == 'STW-Pacote') {
		$found = true;
		break;
	}
}
if (!$found) {
	$config['installedpackages']['package'][] = array(
		'name'              => 'STW-Pacote',
		'descr'             => 'System Way: tema pfSense-Systemway + Backup FTP',
		'version'           => '0.3',
		'configurationfile' => 'stw_pacote.xml'
	);
	write_config("STW-Pacote: registro do pacote (instalacao via script)");
	echo "  -> Pacote registrado no config.xml\n";
} else {
	echo "  -> Pacote ja registrado, atualizando arquivos\n";
}

if (function_exists('stw_pacote_install')) {
	stw_pacote_install();
	echo "  -> Tema aplicado e rotina de backup preparada\n";
}
PHP

# 5) Finalizacao -----------------------------------------------------------
echo "[5/5] Limpando arquivos temporarios..."
rm -rf "${TMP}"

echo ""
echo "======================================================"
echo " Instalacao concluida!"
echo "------------------------------------------------------"
echo " - Tema pfSense-Systemway aplicado (recarregue com Ctrl+F5)"
echo " - Menu: Services > STW Backup FTP (configure o backup e salve)"
echo "======================================================"
exit 0
