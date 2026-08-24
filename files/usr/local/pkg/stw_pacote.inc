<?php
/*
 * stw_pacote.inc  -  Pacote interno System Way (STW-Pacote)  v0.4
 * Copyright (c) 2026 System Way
 */

require_once("config.inc");
require_once("util.inc");
require_once("functions.inc");
require_once("services.inc");
require_once("pkg-utils.inc");

define('STW_THEME_NAME', 'pfSense-Systemway');
define('STW_BACKUP_SCRIPT', '/etc/backup.sh');
define('STW_THEME_SRC', '/usr/local/share/pfSense-pkg-STW-Pacote/theme');
define('STW_THEME_BAK', '/usr/local/share/pfSense-pkg-STW-Pacote/orig_backup');

/* -------------------------------------------------------------------------
 * Mapa de arquivos do tema: origem (staging) => destino no /usr/local/www
 * ------------------------------------------------------------------------- */
function stw_theme_map() {
	return array(
		STW_THEME_SRC . '/css/pfSense-Systemway.css' => '/usr/local/www/css/pfSense-Systemway.css',
		STW_THEME_SRC . '/css/pfSense-dark.css'      => '/usr/local/www/css/pfSense-dark.css',
		STW_THEME_SRC . '/css/login.css'             => '/usr/local/www/css/login.css',
		STW_THEME_SRC . '/js/pfSense.js'             => '/usr/local/www/js/pfSense.js',
		STW_THEME_SRC . '/js/pfSenseHelpers.js'      => '/usr/local/www/js/pfSenseHelpers.js',
		STW_THEME_SRC . '/js/polyfills.js'           => '/usr/local/www/js/polyfills.js',
		STW_THEME_SRC . '/js/traffic-graphs.js'      => '/usr/local/www/js/traffic-graphs.js',
		STW_THEME_SRC . '/www/head.inc'              => '/usr/local/www/head.inc',
		STW_THEME_SRC . '/www/logo.png'              => '/usr/local/www/logo.png',
		STW_THEME_SRC . '/www/logo-branco.png'       => '/usr/local/www/logo-branco.png',
		STW_THEME_SRC . '/www/favicon.ico'           => '/usr/local/www/favicon.ico',
		STW_THEME_SRC . '/www/favicon-16x16.png'     => '/usr/local/www/favicon-16x16.png',
		STW_THEME_SRC . '/www/favicon-32x32.png'     => '/usr/local/www/favicon-32x32.png',
	);
}

function stw_core_files() {
	return array(
		'/usr/local/www/js/pfSense.js',
		'/usr/local/www/js/pfSenseHelpers.js',
		'/usr/local/www/js/polyfills.js',
		'/usr/local/www/js/traffic-graphs.js',
		'/usr/local/www/head.inc',
	);
}

/* -------------------------------------------------------------------------
 * Le a configuracao salva pela GUI (installedpackages/stwpacote/config)
 * ------------------------------------------------------------------------- */
function stw_get_config() {
	global $config;
	if (!is_array($config['installedpackages']['stwpacote']['config'][0])) {
		return array();
	}
	return $config['installedpackages']['stwpacote']['config'][0];
}

/* -------------------------------------------------------------------------
 * Registra o menu em Services (idempotente)
 * ------------------------------------------------------------------------- */
function stw_registrar_menu() {
	global $config;
	if (!is_array($config['installedpackages']['menu'])) {
		$config['installedpackages']['menu'] = array();
	}
	foreach ($config['installedpackages']['menu'] as $it) {
		if (isset($it['name']) && $it['name'] == 'STW Backup FTP') {
			return; // ja existe
		}
	}
	$config['installedpackages']['menu'][] = array(
		'name'        => 'STW Backup FTP',
		'tooltiptext' => 'Backup FTP System Way',
		'section'     => 'Services',
		'url'         => '/pkg_edit.php?xml=stw_pacote.xml'
	);
	log_error("[STW-Pacote] Menu Services > STW Backup FTP registrado.");
}

/* -------------------------------------------------------------------------
 * INSTALL
 * ------------------------------------------------------------------------- */
function stw_pacote_install() {
	stw_deploy_theme();
	stw_aplicar_tema();
	stw_registrar_menu();

	foreach (array("/cf/conf/Relatorio_Diario_Backup.txt",
	               "/cf/conf/Relatorio_Mensal_Backup.txt",
	               STW_BACKUP_SCRIPT) as $f) {
		if (!file_exists($f)) { touch($f); }
	}
	@chmod(STW_BACKUP_SCRIPT, 0777);

	write_config("STW-Pacote: instalacao");
	stw_pacote_resync();
	log_error("[STW-Pacote] Instalacao concluida.");
}

/* -------------------------------------------------------------------------
 * Deploy do tema (com backup dos arquivos core)
 * ------------------------------------------------------------------------- */
function stw_deploy_theme() {
	safe_mkdir(STW_THEME_BAK);
	safe_mkdir('/usr/local/www/css');
	safe_mkdir('/usr/local/www/js');

	foreach (stw_core_files() as $core) {
		$bak = STW_THEME_BAK . '/' . str_replace('/', '_', ltrim($core, '/'));
		if (file_exists($core) && !file_exists($bak)) {
			@copy($core, $bak);
		}
	}
	foreach (stw_theme_map() as $src => $dst) {
		if (file_exists($src)) {
			$dir = dirname($dst);
			if (!is_dir($dir)) { safe_mkdir($dir); }
			@copy($src, $dst);
			@chmod($dst, 0644);
		}
	}
	log_error("[STW-Pacote] Deploy do tema concluido.");
}

function stw_aplicar_tema() {
	global $config;
	if (!is_array($config['system']['webgui'])) {
		$config['system']['webgui'] = array();
	}
	if (empty($config['system']['webgui']['stw_prev_css'])) {
		$config['system']['webgui']['stw_prev_css'] =
			!empty($config['system']['webgui']['webguicss']) ?
			$config['system']['webgui']['webguicss'] : 'pfSense.css';
	}
	$config['system']['webgui']['webguicss'] = STW_THEME_NAME . '.css';
	log_error("[STW-Pacote] Tema '" . STW_THEME_NAME . "' aplicado.");
}

/* -------------------------------------------------------------------------
 * RESYNC  ->  SEMPRE gera o /etc/backup.sh a partir da GUI.
 * O checkbox 'enable' controla apenas o AGENDAMENTO no cron.
 * ------------------------------------------------------------------------- */
function stw_pacote_resync() {
	$cfg = stw_get_config();

	// Re-deploy do tema se solicitado.
	if (!empty($cfg['aplicar_tema'])) {
		stw_deploy_theme();
		stw_aplicar_tema();
	}

	// Garante que o menu exista.
	stw_registrar_menu();

	// Valores (com defaults System Way).
	$host    = !empty($cfg['host'])    ? $cfg['host']    : 'monitoramento.systemway.com.br';
	$porta   = !empty($cfg['porta'])   ? $cfg['porta']   : '21000';
	$usuario = !empty($cfg['usuario']) ? $cfg['usuario'] : 'pfsense';
	$senha   = isset($cfg['senha'])    ? $cfg['senha']   : 'senha';
	$arquivo = !empty($cfg['arquivo']) ? $cfg['arquivo'] : 'firewall.*';

	// SEMPRE gera o script (independente do enable).
	$script = stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo);
	file_put_contents(STW_BACKUP_SCRIPT, $script);
	@chmod(STW_BACKUP_SCRIPT, 0777);
	log_error("[STW-Pacote] /etc/backup.sh gerado (" . strlen($script) . " bytes).");

	// Cron so quando habilitado.
	if (!empty($cfg['enable'])) {
		$hora = (isset($cfg['hora_cron']) && $cfg['hora_cron'] !== '') ? intval($cfg['hora_cron']) : 3;
		stw_agendar_cron($hora);
		log_error("[STW-Pacote] Cron agendado para {$hora}:00.");
	} else {
		stw_remover_cron();
		log_error("[STW-Pacote] Backup gerado, cron NAO agendado (enable desmarcado).");
	}

	write_config("STW-Pacote: resync (backup.sh gerado)");
}

/* -------------------------------------------------------------------------
 * Gera o texto do script (modelo v0.2 - Cleberson A. Tavares)
 * ------------------------------------------------------------------------- */
function stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo) {
	$s  = "#!/bin/sh\n";
	$s .= "#\n";
	$s .= "# Versao 0.4 - Gerado automaticamente pelo pacote STW-Pacote\n";
	$s .= "# Copyright (c) 2026 Systemway\n";
	$s .= "# Base: script v0.2 - Analista de suporte Cleberson A. Tavares\n";
	$s .= "# NAO EDITE MANUALMENTE: use Services > STW Backup FTP\n";
	$s .= "# \$FreeBSD\$\n";
	$s .= "#\n";
	$s .= "########################################################################################\n";
	$s .= "NOME=`hostname`\n";
	$s .= "HOST='"    . $host    . "'\n";
	$s .= "PORTA='"   . $porta   . "'\n";
	$s .= "USUARIO='" . $usuario . "'\n";
	$s .= "SENHA='"   . $senha   . "'\n";
	$s .= "########################################################################################\n\n";
	$s .= "cd /cf/conf\n";
	$s .= "cp config.xml \$NOME-`date +%d_%m_%Y`.xml\n";
	$s .= "echo \" \" > /cf/conf/Relatorio_Diario_Backup.txt\n";
	$s .= "echo \$NOME \"Backup OK\" - `date +%d_%m_%Y` > /cf/conf/Relatorio_Diario_Backup.txt\n";
	$s .= "echo \$NOME \"Backup OK\" - `date +%d_%m_%Y` >> /cf/conf/Relatorio_Mensal_Backup.txt\n";
	$s .= "########################################################################################\n";
	$s .= "# A variavel ARQUIVO e atrelada ao nome do firewall, atente-se ao nome do firewall !!\n";
	$s .= "ARQUIVO='"      . $arquivo . "'\n";
	$s .= "RELATORIO1='Relatorio_Mensal_Backup.txt'\n";
	$s .= "RELATORIO2='Relatorio_Diario_Backup.txt'\n\n";
	$s .= "ftp -n \$HOST \$PORTA <<END_SCRIPT\n";
	$s .= "user \${USUARIO} \${SENHA}\n";
	$s .= "prompt\n";
	$s .= "mput \$ARQUIVO\n";
	$s .= "mput \$RELATORIO1\n";
	$s .= "mput \$RELATORIO2\n";
	$s .= "quit\n";
	$s .= "END_SCRIPT\n\n";
	$s .= "rm -rf /cf/conf/" . $arquivo . "\n\n";
	$s .= "exit 0\n";
	return $s;
}

function stw_agendar_cron($hora) {
	global $config;
	if (!is_array($config['cron']['item'])) {
		$config['cron']['item'] = array();
	}
	foreach ($config['cron']['item'] as $k => $item) {
		if (isset($item['command']) && strpos($item['command'], STW_BACKUP_SCRIPT) !== false) {
			unset($config['cron']['item'][$k]);
		}
	}
	$config['cron']['item'] = array_values($config['cron']['item']);
	$config['cron']['item'][] = array(
		'minute' => '0', 'hour' => (string)$hora, 'mday' => '*',
		'month' => '*', 'wday' => '*', 'who' => 'root',
		'command' => '/bin/sh ' . STW_BACKUP_SCRIPT
	);
	configure_cron();
}

function stw_remover_cron() {
	global $config;
	if (!is_array($config['cron']['item'])) { return; }
	$changed = false;
	foreach ($config['cron']['item'] as $k => $item) {
		if (isset($item['command']) && strpos($item['command'], STW_BACKUP_SCRIPT) !== false) {
			unset($config['cron']['item'][$k]);
			$changed = true;
		}
	}
	if ($changed) {
		$config['cron']['item'] = array_values($config['cron']['item']);
		configure_cron();
	}
}

/* -------------------------------------------------------------------------
 * DEINSTALL
 * ------------------------------------------------------------------------- */
function stw_pacote_deinstall() {
	global $config;
	stw_remover_cron();

	foreach (stw_core_files() as $core) {
		$bak = STW_THEME_BAK . '/' . str_replace('/', '_', ltrim($core, '/'));
		if (file_exists($bak)) {
			@copy($bak, $core);
			@unlink($bak);
		}
	}
	foreach (array(
		'/usr/local/www/css/pfSense-Systemway.css',
		'/usr/local/www/css/pfSense-dark.css',
		'/usr/local/www/css/login.css',
		'/usr/local/www/logo.png',
		'/usr/local/www/logo-branco.png',
		'/usr/local/www/favicon.ico',
		'/usr/local/www/favicon-16x16.png',
		'/usr/local/www/favicon-32x32.png',
	) as $f) {
		unlink_if_exists($f);
	}
	if (!empty($config['system']['webgui']['stw_prev_css'])) {
		$config['system']['webgui']['webguicss'] = $config['system']['webgui']['stw_prev_css'];
		unset($config['system']['webgui']['stw_prev_css']);
	}
	// remove menu
	if (is_array($config['installedpackages']['menu'])) {
		foreach ($config['installedpackages']['menu'] as $k => $it) {
			if (isset($it['name']) && $it['name'] == 'STW Backup FTP') {
				unset($config['installedpackages']['menu'][$k]);
			}
		}
		$config['installedpackages']['menu'] = array_values($config['installedpackages']['menu']);
	}
	unlink_if_exists(STW_BACKUP_SCRIPT);
	write_config("STW-Pacote: desinstalacao");
	log_error("[STW-Pacote] Desinstalacao concluida.");
}
