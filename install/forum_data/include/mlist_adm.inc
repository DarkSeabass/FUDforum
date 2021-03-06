<?php
/**
* copyright            : (C) 2001-2013 Advanced Internet Designs Inc.
* email                : forum@prohost.org
* $Id: mlist_adm.inc 5704 2013-10-06 15:23:42Z naudefj $
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 2 of the License.
**/

class fud_mlist
{
	var $id, $forum_id, $name, $custom_sig;
	var $mlist_opt=104;
	var $subject_regex_haystack, $subject_regex_needle, $body_regex_haystack, $body_regex_needle, $additional_headers;
	var $mbox_server, $mbox_user, $mbox_pass, $mbox_type=0;
	var $fixed_from_address;

	function __construct()
	{
		if (isset($_POST) && count($_POST)) {
			$el = array_keys(get_object_vars($this));
			foreach ($el as $v) {
		        	if (isset($_POST['ml_'. $v])) {
					$this->{$v} = $_POST['ml_'. $v];
				}
			}

			// Text fields.
			foreach (array('custom_sig', 'additional_headers', 'mbox_server', 'mbox_user', 'fixed_from_address') as $v) {
				$this->$v = trim($this->$v);
			}

			$this->mlist_opt = 0;
			foreach (array('skip_non_forum_users', 'mlist_post_apr', 'allow_frm_post', 'frm_post_apr', 'allow_mlist_attch', 'allow_mlist_html', 'complex_reply_match', 'create_users') as $v) {
				if (isset($_POST['ml_'. $v])) {
					$this->mlist_opt |= (int) $_POST['ml_'. $v];
				}
			}

			if (isset($_POST['ml_mbox_type'])) {
				$this->mbox_type |= (int) $_POST['ml_mbox_type'];
			}

			if ($this->subject_regex_haystack) {
				$this->subject_regex_haystack = '/'. $this->subject_regex_haystack .'/'. $_POST['ml_subject_regex_haystack_opt'];
			}
			if ($this->body_regex_haystack) {
				$this->body_regex_haystack = '/'. $this->body_regex_haystack .'/'. $_POST['ml_body_regex_haystack_opt'];
			}
		}
	}

	/** Add new mailing list rule. */
	function add()
	{
		if (!db_locked()) {
			$ll = 1;
			db_lock($GLOBALS['DBHOST_TBL_PREFIX'] .'forum WRITE, '. $GLOBALS['DBHOST_TBL_PREFIX'] .'mlist WRITE');
		}
		$this->id = db_qid('INSERT INTO '. $GLOBALS['DBHOST_TBL_PREFIX'] .'mlist(
			forum_id,
			name,
			mlist_opt,
			subject_regex_haystack,
			subject_regex_needle,
			body_regex_haystack,
			body_regex_needle,
			additional_headers,
			custom_sig,
			mbox_server,
			mbox_user, 
			mbox_pass, 
			mbox_type,
			fixed_from_address
			)
			VALUES(
			'. $this->forum_id .',
			'. _esc($this->name) .',
			'. $this->mlist_opt .',
			'. _esc($this->subject_regex_haystack) .',
			'. _esc($this->subject_regex_needle) .',
			'. _esc($this->body_regex_haystack) .',
			'. _esc($this->body_regex_needle) .',
			'. _esc($this->additional_headers) .',
			'. _esc($this->custom_sig) .',
			'. _esc($this->mbox_server) .',
			'. _esc($this->mbox_user) .',
			'. _esc($this->mbox_pass) .',
			'. $this->mbox_type .',
			'. _esc($this->fixed_from_address) .'
			)');
		if ($this->mlist_opt & 4) {
			q('UPDATE '. $GLOBALS['DBHOST_TBL_PREFIX'] .'forum SET forum_opt='. q_bitor('forum_opt', 2) .' WHERE id='. $this->forum_id);
		}
		if (isset($ll)) {
			db_unlock();
		}
		return $this->id;
	}

	/** Update existing mailing list rule. */
	function sync($id)
	{
		if (!db_locked()) {
			$ll = 1;
			db_lock($GLOBALS['DBHOST_TBL_PREFIX'] .'forum WRITE, '. $GLOBALS['DBHOST_TBL_PREFIX'] .'mlist WRITE');
		}
		list($old_frm_id, $mlist_opt) = db_saq('SELECT forum_id, mlist_opt FROM '. $GLOBALS['DBHOST_TBL_PREFIX'] .'mlist WHERE id='. $id);

		q('UPDATE '.$GLOBALS['DBHOST_TBL_PREFIX'] .'mlist SET
			forum_id='. $this->forum_id .',
			name='. _esc($this->name) .',
			mlist_opt='. $this->mlist_opt .',
			subject_regex_haystack='. _esc($this->subject_regex_haystack) .',
			subject_regex_needle='. _esc($this->subject_regex_needle) .',
			body_regex_haystack='. _esc($this->body_regex_haystack) .',
			body_regex_needle='. _esc($this->body_regex_needle) .',
			additional_headers='. _esc($this->additional_headers) .',
			custom_sig='. _esc($this->custom_sig) .',
			mbox_server='. _esc($this->mbox_server) .',
			mbox_user='. _esc($this->mbox_user) .',
			mbox_pass='. _esc($this->mbox_pass) .',
			mbox_type='. $this->mbox_type .',
			fixed_from_address ='. _esc($this->fixed_from_address) .'
		WHERE id='. $id);

		if ($old_frm_id != $this->forum_id && $mlist_opt & 4) {
			q('UPDATE '. $GLOBALS['DBHOST_TBL_PREFIX'] .'forum SET forum_opt='. q_bitor('forum_opt', ~2) .' WHERE id='. $old_frm_id);
		}
		q('UPDATE '. $GLOBALS['DBHOST_TBL_PREFIX'] .'forum SET forum_opt='. q_bitor('forum_opt', ($this->mlist_opt & 1 ? 2 : 0)) .' WHERE id='. $this->forum_id);

		if (isset($ll)) {
			db_unlock();
		}
	}

	/** Remove a mailing list rule. */
	function del($id)
	{
		q('DELETE FROM '. $GLOBALS['DBHOST_TBL_PREFIX'] .'mlist WHERE id='. $id);
	}
}

?>
