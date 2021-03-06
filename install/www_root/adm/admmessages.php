<?php
/**
* copyright            : (C) 2001-2013 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id: admmessages.php 5692 2013-09-27 20:29:48Z naudefj $
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 2 of the License.
**/

	require('./GLOBALS.php');
	fud_use('adm.inc', true);

	if (isset($_POST['tname'], $_POST['tlang'], $_POST['btn_edit'])) {
		header('Location: msglist.php?tname='. $_POST['tname'] .'&tlang='. $_POST['tlang'] .'&'. __adm_rsidl);
		exit;
	}

	require($WWW_ROOT_DISK .'adm/header.php');

	if (isset($_POST['btn_download']) && isset($_POST['tlang'])) {
		$tlang = $_POST['tlang'];
		pf(successify('Downloading '. $tlang .' messages from translatewiki.net...'));

		fud_use('url.inc', true);	// For get_remote_file().
		$url = 'http://translatewiki.net/w/i.php?title=Special%3ATranslate&task=export-to-file&group=out-fudforum&language='. $tlang .'&limit=2500';
		$messages = get_remote_file($url);

		if (!strlen($messages)) {
			echo errorify('Download failed. Your connection might be down or a firewall or proxy is blocking access.');
		} elseif ( substr($messages, 0, 15) != '# Messages for ' ) {
			echo errorify('Corrupted download. Please try again.');
		} else {
			$msgfile = $GLOBALS['DATA_DIR'] .'thm/default/i18n/'. $tlang .'/msg';
			file_put_contents($msgfile, $messages);
		
			// Rebuild themes based on this language.
			fud_use('compiler.inc', true);
			$c = q('SELECT theme, name FROM '. $GLOBALS['DBHOST_TBL_PREFIX'] .'themes WHERE lang='. _esc($tlang));
			while ($r = db_rowarr($c)) {
				compile_all($r[0], $tlang, $r[1]);
				echo successify('Theme '. $r[0] .' ('. $tlang .') was successfully rebuilt.');
			}
			unset($c);
		}
	}

	list($def_thm, $def_tmpl) = db_saq('SELECT name, lang FROM '. $GLOBALS['DBHOST_TBL_PREFIX'] .'themes WHERE theme_opt=3');
?>
<h2>Message Editor</h2>
<div class="tutor">
	Please use <a href="http://translatewiki.net/wiki/Translating:FUDforum">TranslateWiki.net</a> to translate FUDforum's messages to other languages.
	Only edit messages to make site specific languages updates or to apply temporary translation changes (use a <a href="admtemplates.php?<?php echo __adm_rsid; ?>">custom theme</a> to prevent future upgrades from overwriting your changes).
</div>

<h3>Edit messages:</h3>
<p>Select a template set and language to edit:</p>
<form method="post" action="admmessages.php">
<?php echo _hs; ?>
<table class="datatable solidtable">
<tr class="field">
<td>Template Set:</td><td><select name="tname">
<?php
	foreach (glob($GLOBALS['DATA_DIR'] .'/thm/*', GLOB_ONLYDIR) as $file) {
		if (!file_exists($file .'/tmpl')) {
			continue;
		}
		$n = basename($file);
		echo '<option value="'. $n .'"'. ($n == $def_thm ? ' selected="selected"' : '') .'>'. $n .'</option>';
	}
?>
</select></td>
</tr>
<tr class="field">
<td>Language:</td><td><select name="tlang">
<?php
	foreach (glob($GLOBALS['DATA_DIR'] .'thm/default/i18n/*', GLOB_ONLYDIR) as $file) {
		$langcode = $langname = basename($file);
		if (!file_exists($file .'/msg') || $langcode == 'qqq') {
			continue;	// No messages or tranlations tips.
		}
		if (file_exists($file .'/name')) {
			$langname = trim(file_get_contents($file .'/name'));
		}
		echo '<option value="'. $langcode .'"'.($langcode == $def_tmpl ? ' selected="selected"' : '').'>'. $langname .'</option>';
	}
?>
</select></td></tr>
<tr class="fieldaction" align="right"><td colspan="2"><input type="submit" name="btn_edit" value="Edit" /></td></tr></table></form>

<h3>Download latest translations:</h3>
<?php
	$disabled = ((bool)ini_get('allow_url_fopen')) ? '' : 'disabled="disabled"';
	if ($disabled) {
		echo '<p>Your PHP installation does not allow downloading of files from the Internet. Enable <b>allow_url_fopen</b> in <b>php.ini</b> to use this feature.</p>';
	} else {
		echo '<p>Download the latest default tanslations from the <a href="http://translatewiki.net/wiki/Translating:FUDforum">translatewiki.net</a> website:</p>';
	}
?>
<form method="post" action="admmessages.php">
<?php echo _hs; ?>
<table class="datatable solidtable">
<tr class="field">
<td>Language:</td><td><select name="tlang">
<?php
	foreach (glob($GLOBALS['DATA_DIR'] .'thm/default/i18n/*', GLOB_ONLYDIR) as $file) {
		if (!file_exists($file .'/msg')) {
			continue;
		}
		$n = basename($file);
		if ($n == 'en') {
			continue; // No translations, English is the primary language.
		}
		echo '<option value="'. $n .'"'.($n == $def_tmpl ? ' selected="selected"' : '').'>'. $n .'</option>';
	}
?>
</select></td></tr>
<tr class="fieldaction" align="right"><td colspan="2"><input type="submit" name="btn_download" value="Download" <?php echo $disabled; ?> /></td></tr></table></form>

<?php require($WWW_ROOT_DISK .'adm/footer.php'); ?>
