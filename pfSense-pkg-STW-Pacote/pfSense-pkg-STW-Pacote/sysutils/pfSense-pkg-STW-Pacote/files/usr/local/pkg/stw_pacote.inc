<?php
/*
 * stw_pacote.inc
 * Pacote interno System Way (STW-Pacote)
 * Copyright (c) 2026 System Way
 *
 * Funcoes:
 *   - stw_pacote_install()   -> aplica tema STW (deploy nos caminhos do www) + resync
 *   - stw_pacote_deinstall() -> remove cron, restaura arquivos core originais e tema padrao
 *   - stw_pacote_resync()    -> gera /etc/backup.sh a partir da GUI e agenda cron
 */

require_once("config.inc");
require_once("util.inc");
require_once("functions.inc");
require_once("services.inc");
require_once("pkg-utils.inc");

/* Nome do tema (css) aplicado no webGUI */
define('STW_THEME_NAME', 'pfSense-Systemway');
define('STW_BACKUP_SCRIPT', '/etc/backup.sh');
/* Diretorio de staging (assets do tema entregues pelo pacote) */
define('STW_THEME_SRC', '/usr/local/share/pfSense-pkg-STW-Pacote/theme');
/* Onde guardamos o backup dos arquivos core originais */
define('STW_THEME_BAK', '/usr/local/share/pfSense-pkg-STW-Pacote/orig_backup');

/* ---------------------------------------------------------------------------
 * Mapa: arquivo de origem (staging) => destino no /usr/local/www
 * Arquivos CORE (head.inc, *.js) sao respaldados antes de sobrescrever.
 * ------------------------------------------------------------------------- */
function stw_theme_map() {
	return array(
		// CSS do tema (nao existem por padrao, nao precisam de backup)
		STW_THEME_SRC . '/css/pfSense-Systemway.css' => '/usr/local/www/css/pfSense-Systemway.css',
		STW_THEME_SRC . '/css/pfSense-dark.css'      => '/usr/local/www/css/pfSense-dark.css',
		STW_THEME_SRC . '/css/login.css'             => '/usr/local/www/css/login.css',
		// JS core (SERAO respaldados)
		STW_THEME_SRC . '/js/pfSense.js'             => '/usr/local/www/js/pfSense.js',
		STW_THEME_SRC . '/js/pfSenseHelpers.js'      => '/usr/local/www/js/pfSenseHelpers.js',
		STW_THEME_SRC . '/js/polyfills.js'           => '/usr/local/www/js/polyfills.js',
		STW_THEME_SRC . '/js/traffic-graphs.js'      => '/usr/local/www/js/traffic-graphs.js',
		// head.inc core (SERA respaldado)
		STW_THEME_SRC . '/www/head.inc'              => '/usr/local/www/head.inc',
		// Binarios (logos/favicons) - so copiam se o usuario os incluiu no staging
		STW_THEME_SRC . '/www/logo.png'              => '/usr/local/www/logo.png',
		STW_THEME_SRC . '/www/logo-branco.png'       => '/usr/local/www/logo-branco.png',
		STW_THEME_SRC . '/www/favicon.ico'           => '/usr/local/www/favicon.ico',
		STW_THEME_SRC . '/www/favicon-16x16.png'     => '/usr/local/www/favicon-16x16.png',
		STW_THEME_SRC . '/www/favicon-32x32.png'     => '/usr/local/www/favicon-32x32.png',
	);
}

/* Arquivos core que devem ter backup antes de sobrescrever */
function stw_core_files() {
	return array(
		'/usr/local/www/js/pfSense.js',
		'/usr/local/www/js/pfSenseHelpers.js',
		'/usr/local/www/js/polyfills.js',
		'/usr/local/www/js/traffic-graphs.js',
		'/usr/local/www/head.inc',
	);
}

/* ---------------------------------------------------------------------------
 * Helper: retorna o array de configuracao do pacote
 * ------------------------------------------------------------------------- */
function stw_get_config() {
	global $config;
	if (!is_array($config['installedpackages']['stwpacote'])) {
		$config['installedpackages']['stwpacote'] = array();
	}
	if (!is_array($config['installedpackages']['stwpacote']['config'])) {
		$config['installedpackages']['stwpacote']['config'] = array();
	}
	return $config['installedpackages']['stwpacote']['config'][0];
}

/* ---------------------------------------------------------------------------
 * INSTALL: aplica tema (deploy) + prepara rotina de backup
 * ------------------------------------------------------------------------- */
function stw_pacote_install() {
	global $config;

	stw_deploy_theme();
	stw_aplicar_tema();

	// Arquivos padrao da rotina de backup
	$files = array(
		"/cf/conf/Relatorio_Diario_Backup.txt",
		"/cf/conf/Relatorio_Mensal_Backup.txt",
		STW_BACKUP_SCRIPT
	);
	foreach ($files as $f) {
		if (!file_exists($f)) {
			touch($f);
		}
	}
	@chmod(STW_BACKUP_SCRIPT, 0777);

	stw_pacote_resync();

	log_error("[STW-Pacote] Instalacao concluida: tema pfSense-Systemway aplicado e rotina de backup preparada.");
}

/* ---------------------------------------------------------------------------
 * Copia os assets do tema para /usr/local/www, com backup dos arquivos core
 * ------------------------------------------------------------------------- */
function stw_deploy_theme() {
	// Garante diretorio de backup
	safe_mkdir(STW_THEME_BAK);
	safe_mkdir('/usr/local/www/css');
	safe_mkdir('/usr/local/www/js');

	// 1) Backup dos arquivos core originais (apenas 1x)
	foreach (stw_core_files() as $core) {
		$bak = STW_THEME_BAK . '/' . str_replace('/', '_', ltrim($core, '/'));
		if (file_exists($core) && !file_exists($bak)) {
			@copy($core, $bak);
			log_error("[STW-Pacote] Backup do arquivo core: {$core}");
		}
	}

	// 2) Copia os arquivos do tema para os destinos
	foreach (stw_theme_map() as $src => $dst) {
		if (file_exists($src)) {
			$dstdir = dirname($dst);
			if (!is_dir($dstdir)) {
				safe_mkdir($dstdir);
			}
			@copy($src, $dst);
			@chmod($dst, 0644);
		} else {
			// Binarios ausentes: apenas registra aviso (nao interrompe).
			if (strpos($src, '/www/') !== false &&
			    (substr($src, -4) === '.png' || substr($src, -4) === '.ico')) {
				log_error("[STW-Pacote] AVISO: asset ausente no staging: {$src} (coloque o binario e reinstale).");
			}
		}
	}
	log_error("[STW-Pacote] Deploy dos arquivos de tema concluido.");
}

/* ---------------------------------------------------------------------------
 * Aplica o tema personalizado STW no webGUI (webguicss)
 * ------------------------------------------------------------------------- */
function stw_aplicar_tema() {
	global $config;

	if (!is_array($config['system'])) {
		$config['system'] = array();
	}
	if (!is_array($config['system']['webgui'])) {
		$config['system']['webgui'] = array();
	}

	if (empty($config['system']['webgui']['stw_prev_css'])) {
		$config['system']['webgui']['stw_prev_css'] =
			!empty($config['system']['webgui']['webguicss']) ?
			$config['system']['webgui']['webguicss'] : 'pfSense.css';
	}

	$config['system']['webgui']['webguicss'] = STW_THEME_NAME . '.css';

	write_config("STW-Pacote: tema personalizado aplicado");
	log_error("[STW-Pacote] Tema '" . STW_THEME_NAME . "' aplicado no webGUI.");
}

/* ---------------------------------------------------------------------------
 * RESYNC: gera /etc/backup.sh a partir dos campos da GUI + agenda cron
 * ------------------------------------------------------------------------- */
function stw_pacote_resync() {
	global $config;

	$cfg = stw_get_config();

	// Reaplica/re-deploy o tema se a checkbox estiver marcada.
	if (!empty($cfg['aplicar_tema'])) {
		stw_deploy_theme();
		stw_aplicar_tema();
	}

	if (empty($cfg['enable'])) {
		stw_remover_cron();
		log_error("[STW-Pacote] Rotina de backup desabilitada.");
		return;
	}

	$host    = !empty($cfg['host'])    ? $cfg['host']    : 'monitoramento.systemway.com.br';
	$porta   = !empty($cfg['porta'])   ? $cfg['porta']   : '21000';
	$usuario = !empty($cfg['usuario']) ? $cfg['usuario'] : 'pfsense';
	$senha   = isset($cfg['senha'])    ? $cfg['senha']   : 'senha';
	$arquivo = !empty($cfg['arquivo']) ? $cfg['arquivo'] : 'firewall.*';

	$script = stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo);

	file_put_contents(STW_BACKUP_SCRIPT, $script);
	@chmod(STW_BACKUP_SCRIPT, 0777);

	$hora = isset($cfg['hora_cron']) && $cfg['hora_cron'] !== '' ? intval($cfg['hora_cron']) : 3;
	stw_agendar_cron($hora);

	log_error("[STW-Pacote] /etc/backup.sh gerado e cron agendado para {$hora}:00.");
}

/* ---------------------------------------------------------------------------
 * Gera o texto do script de backup (baseado no modelo v0.2 - Cleberson Tavares)
 * ------------------------------------------------------------------------- */
function stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo) {
	$s  = "#!/bin/sh\n";
	$s .= "#\n";
	$s .= "# Versao 0.3 - Gerado automaticamente pelo pacote STW-Pacote\n";
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

/* ---------------------------------------------------------------------------
 * Agenda / atualiza a entrada no cron do pfSense
 * ------------------------------------------------------------------------- */
function stw_agendar_cron($hora) {
	global $config;

	if (!is_array($config['cron'])) {
		$config['cron'] = array();
	}
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
		'minute'  => '0',
		'hour'    => (string)$hora,
		'mday'    => '*',
		'month'   => '*',
		'wday'    => '*',
		'who'     => 'root',
		'command' => '/bin/sh ' . STW_BACKUP_SCRIPT
	);

	write_config("STW-Pacote: agendamento de backup FTP");
	configure_cron();
}

/* ---------------------------------------------------------------------------
 * Remove a entrada do cron do STW
 * ------------------------------------------------------------------------- */
function stw_remover_cron() {
	global $config;

	if (!is_array($config['cron']) || !is_array($config['cron']['item'])) {
		return;
	}
	$changed = false;
	foreach ($config['cron']['item'] as $k => $item) {
		if (isset($item['command']) && strpos($item['command'], STW_BACKUP_SCRIPT) !== false) {
			unset($config['cron']['item'][$k]);
			$changed = true;
		}
	}
	if ($changed) {
		$config['cron']['item'] = array_values($config['cron']['item']);
		write_config("STW-Pacote: remocao do agendamento de backup FTP");
		configure_cron();
	}
}

/* ---------------------------------------------------------------------------
 * DEINSTALL: remove cron, restaura arquivos core originais e tema anterior
 * ------------------------------------------------------------------------- */
function stw_pacote_deinstall() {
	global $config;

	stw_remover_cron();

	// Restaura arquivos core originais (head.inc, *.js) a partir do backup.
	foreach (stw_core_files() as $core) {
		$bak = STW_THEME_BAK . '/' . str_replace('/', '_', ltrim($core, '/'));
		if (file_exists($bak)) {
			@copy($bak, $core);
			@unlink($bak);
			log_error("[STW-Pacote] Arquivo core restaurado: {$core}");
		}
	}

	// Remove os CSS/binarios adicionados pelo tema.
	$remove = array(
		'/usr/local/www/css/pfSense-Systemway.css',
		'/usr/local/www/css/pfSense-dark.css',
		'/usr/local/www/css/login.css',
		'/usr/local/www/logo.png',
		'/usr/local/www/logo-branco.png',
		'/usr/local/www/favicon.ico',
		'/usr/local/www/favicon-16x16.png',
		'/usr/local/www/favicon-32x32.png',
	);
	foreach ($remove as $f) {
		unlink_if_exists($f);
	}

	// Restaura tema anterior.
	if (!empty($config['system']['webgui']['stw_prev_css'])) {
		$config['system']['webgui']['webguicss'] = $config['system']['webgui']['stw_prev_css'];
		unset($config['system']['webgui']['stw_prev_css']);
		write_config("STW-Pacote: tema anterior restaurado");
	}

	unlink_if_exists(STW_BACKUP_SCRIPT);

	log_error("[STW-Pacote] Desinstalacao concluida.");
}
