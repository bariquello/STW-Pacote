<?php
/*
 * stw_pacote.inc  -  Pacote interno System Way (STW-Pacote)  v0.6
 * Copyright (c) 2026 System Way
 *
 * v0.6: instala dependencias (Cron, Zabbix Agent 5.0, Zabbix Proxy 5.0) de forma
 *       IDEMPOTENTE (verifica antes, nao reinstala) e provisiona o script
 *       pfsense_zbx.php em /root/scripts.
 * v0.5: corrige leitura da configuracao salva pela GUI.
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

/* Script de monitoramento pfSense para Zabbix */
define('STW_ZBX_DIR',  '/root/scripts');
define('STW_ZBX_FILE', '/root/scripts/pfsense_zbx.php');
define('STW_ZBX_URL',  'https://raw.githubusercontent.com/rbicelli/pfsense-zabbix-template/master/pfsense_zbx.php');

/* -------------------------------------------------------------------------
 * DEPENDENCIAS: pacotes oficiais do repositorio pfSense
 * ------------------------------------------------------------------------- */
function stw_dependencias() {
	return array(
		'pfSense-pkg-Cron'          => 'Cron',
		'pfSense-pkg-zabbix-agent5' => 'Zabbix Agent 5.0',
		'pfSense-pkg-zabbix-proxy5' => 'Zabbix Proxy 5.0',
	);
}

/* Verifica se um pacote ja esta instalado (pkg info -e retorna 0 se existe) */
function stw_pkg_instalado($pkg) {
	$rc = 0;
	$out = array();
	exec("/usr/local/sbin/pkg-static info -e " . escapeshellarg($pkg) . " 2>/dev/null", $out, $rc);
	if ($rc === 0) {
		return true;
	}
	// Fallback: consulta o registro de pacotes do proprio pfSense.
	$curto = preg_replace('/^pfSense-pkg-/', '', $pkg);
	global $config;
	if (is_array($config['installedpackages']['package'])) {
		foreach ($config['installedpackages']['package'] as $p) {
			if (isset($p['name']) && strcasecmp($p['name'], $curto) == 0) {
				return true;
			}
		}
	}
	return false;
}

/*
 * Instala as dependencias que ainda nao existirem.
 * Retorna array com o resumo do que foi feito.
 */
function stw_instalar_dependencias() {
	$resumo = array();

	foreach (stw_dependencias() as $pkg => $label) {
		if (stw_pkg_instalado($pkg)) {
			$resumo[] = "[=] {$label}: ja instalado, nada a fazer";
			log_error("[STW-Pacote] {$label} ja instalado - ignorando.");
			continue;
		}

		log_error("[STW-Pacote] Instalando {$label} ({$pkg})...");
		$rc = 0; $out = array();
		exec("/usr/local/sbin/pkg-static install -y " . escapeshellarg($pkg) . " 2>&1", $out, $rc);

		if ($rc === 0 && stw_pkg_instalado($pkg)) {
			$resumo[] = "[+] {$label}: instalado com sucesso";
			log_error("[STW-Pacote] {$label} instalado com sucesso.");
		} else {
			$resumo[] = "[!] {$label}: FALHA na instalacao (verifique DNS/internet)";
			log_error("[STW-Pacote] FALHA ao instalar {$label}: " . implode(' | ', array_slice($out, -3)));
		}
	}

	// Recarrega o registro de pacotes para que os menus apareçam.
	if (function_exists('sync_package_all')) {
		@sync_package_all();
	}

	return $resumo;
}

/* -------------------------------------------------------------------------
 * Provisiona o script pfsense_zbx.php em /root/scripts
 * Equivale a: mkdir /root/scripts && curl -o /root/scripts/pfsense_zbx.php <URL>
 * ------------------------------------------------------------------------- */
function stw_instalar_zbx_script($forcar = false) {
	if (!is_dir(STW_ZBX_DIR)) {
		safe_mkdir(STW_ZBX_DIR, 0755);
		log_error("[STW-Pacote] Diretorio " . STW_ZBX_DIR . " criado.");
	}

	if (file_exists(STW_ZBX_FILE) && filesize(STW_ZBX_FILE) > 0 && !$forcar) {
		log_error("[STW-Pacote] pfsense_zbx.php ja existe - ignorando download.");
		return "[=] pfsense_zbx.php: ja existe, nada a fazer";
	}

	$rc = 0; $out = array();
	// fetch e nativo do pfSense; curl usado como alternativa se disponivel.
	exec("/usr/bin/fetch -q -o " . escapeshellarg(STW_ZBX_FILE) . " " . escapeshellarg(STW_ZBX_URL) . " 2>&1", $out, $rc);
	if ($rc !== 0 || !file_exists(STW_ZBX_FILE) || filesize(STW_ZBX_FILE) == 0) {
		exec("/usr/local/bin/curl -sSL -o " . escapeshellarg(STW_ZBX_FILE) . " " . escapeshellarg(STW_ZBX_URL) . " 2>&1", $out, $rc);
	}

	if (file_exists(STW_ZBX_FILE) && filesize(STW_ZBX_FILE) > 0) {
		@chmod(STW_ZBX_FILE, 0755);
		log_error("[STW-Pacote] pfsense_zbx.php baixado (" . filesize(STW_ZBX_FILE) . " bytes).");
		return "[+] pfsense_zbx.php: baixado em " . STW_ZBX_FILE;
	}

	log_error("[STW-Pacote] FALHA ao baixar pfsense_zbx.php.");
	return "[!] pfsense_zbx.php: FALHA no download";
}

/* -------------------------------------------------------------------------
 * Mapa de arquivos do tema
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
 * LEITURA ROBUSTA DA CONFIGURACAO
 * ------------------------------------------------------------------------- */
function stw_get_config() {
	global $config;

	if (!is_array($config['installedpackages'])) {
		return array();
	}
	$ip = $config['installedpackages'];

	$candidatos = array('stwpacote', 'stwbackupftp', 'stw_backup_ftp', 'stwbackup', 'stwpkg');
	foreach ($candidatos as $k) {
		if (isset($ip[$k]['config'][0]) && is_array($ip[$k]['config'][0]) && count($ip[$k]['config'][0]) > 0) {
			return $ip[$k]['config'][0];
		}
		if (isset($ip[$k]['config']) && is_array($ip[$k]['config']) && isset($ip[$k]['config']['host'])) {
			return $ip[$k]['config'];
		}
	}

	foreach ($ip as $chave => $val) {
		if (!is_array($val)) { continue; }
		if (isset($val['config'][0]) && is_array($val['config'][0])) {
			$c = $val['config'][0];
			if (isset($c['host']) || isset($c['usuario']) || isset($c['senha'])) {
				log_error("[STW-Pacote] Config localizada em installedpackages/{$chave}/config/0");
				return $c;
			}
		}
	}

	return array();
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
			return;
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
 * Ordem: dependencias (Cron + Zabbix) -> script zbx -> tema -> menu -> backup
 * ------------------------------------------------------------------------- */
function stw_pacote_install() {
	// 1) Dependencias primeiro (idempotente).
	$resumo = stw_instalar_dependencias();
	foreach ($resumo as $linha) { echo "  {$linha}\n"; }

	// 2) Script de monitoramento do Zabbix.
	echo "  " . stw_instalar_zbx_script(false) . "\n";

	// 3) Tema + menu.
	stw_deploy_theme();
	stw_aplicar_tema();
	stw_registrar_menu();

	// 4) Arquivos da rotina de backup.
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
 * ------------------------------------------------------------------------- */
function stw_pacote_resync() {
	$cfg = stw_get_config();

	if (!empty($cfg['aplicar_tema'])) {
		stw_deploy_theme();
		stw_aplicar_tema();
	}

	// Reinstalar/verificar dependencias sob demanda pela GUI.
	if (!empty($cfg['instalar_deps'])) {
		stw_instalar_dependencias();
		stw_instalar_zbx_script(false);
	}

	stw_registrar_menu();

	$host    = html_entity_decode(!empty($cfg['host'])    ? $cfg['host']    : 'monitoramento.systemway.com.br', ENT_QUOTES);
	$porta   = html_entity_decode(!empty($cfg['porta'])   ? $cfg['porta']   : '21000', ENT_QUOTES);
	$usuario = html_entity_decode(!empty($cfg['usuario']) ? $cfg['usuario'] : 'pfsense', ENT_QUOTES);
	$senha   = html_entity_decode(isset($cfg['senha'])    ? $cfg['senha']   : 'senha', ENT_QUOTES);
	$arquivo = html_entity_decode(!empty($cfg['arquivo']) ? $cfg['arquivo'] : 'firewall.*', ENT_QUOTES);

	$script = stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo);
	file_put_contents(STW_BACKUP_SCRIPT, $script);
	@chmod(STW_BACKUP_SCRIPT, 0777);
	log_error("[STW-Pacote] /etc/backup.sh gerado para usuario '{$usuario}' em '{$host}'.");

	if (!empty($cfg['enable'])) {
		$hora = (isset($cfg['hora_cron']) && $cfg['hora_cron'] !== '') ? intval($cfg['hora_cron']) : 3;
		stw_agendar_cron($hora);
	} else {
		stw_remover_cron();
	}

	write_config("STW-Pacote: resync (backup.sh gerado)");
}

/* -------------------------------------------------------------------------
 * Geracao do script (modelo v0.2 - Cleberson A. Tavares)
 * ------------------------------------------------------------------------- */
function stw_sh_quote($v) {
	return str_replace("'", "'\\''", $v);
}

function stw_gerar_backup_script($host, $porta, $usuario, $senha, $arquivo) {
	$host    = stw_sh_quote($host);
	$porta   = stw_sh_quote($porta);
	$usuario = stw_sh_quote($usuario);
	$senha   = stw_sh_quote($senha);
	$arquivo = stw_sh_quote($arquivo);

	$s  = "#!/bin/sh\n";
	$s .= "#\n";
	$s .= "# Versao 0.6 - Gerado automaticamente pelo pacote STW-Pacote\n";
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
 * Obs.: NAO removemos Cron/Zabbix nem o pfsense_zbx.php, pois podem estar
 *       em uso por outras rotinas do cliente.
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
