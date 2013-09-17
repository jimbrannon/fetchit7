<?php

/**
 * @file
 * Forms page callbacks for the fetchit7 module
 */

include_once(drupal_get_path('module', 'fetchit7').'/fetchit7.constants.inc'); 

//===================================
// fetchit7 html table form content
//===================================

/**
  * Form builder.  Display fetchit7 html table content.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_table_form($form, &$form_state) {

	// get nid embedded into original form_id; format fetchit7_table_form_X### - nid starts at 21
	$form_id = $form_state['build_info']['form_id'];
	$nid = (int) substr($form_id, 21);

	// get database type and store it (hidden) in the form so it is available to the form validate and submit functions
	$db_type = trim(fetchit7_content_field_fetcher($nid,'db_type','value'));
	if(strlen($db_type)) {
		//convert it to a number so it can work with old fetchit2 code...
		//TODO: convert fetchit2 based code to use a string db_type
		if (!is_null($db_type_number = array_search($db_type, array(0 => 'postgresql', 1 => 'mysql', 2 => 'sqlserver')))) {
			$form['db_type'] = array('#type' => 'value','#value' => $db_type_number);
		} else {
			drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the database type.",'error');
			return $form;
		}
	} else {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the database type.",'error');
		return $form;
	}

	// get database handle
	if ($db_handle = fetchit_get_db_handle($nid)) {
		$form['db_handle'] = array('#type' => 'value','#value' => $db_handle);
	} else {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when creating the database connection handle.",'error');
		return $form;
	}
	
	// run the pre query before doing anything else
	// TODO: figure out why pre-fn is getting hit twice prior to the form drawing!!!
	$preqry = trim(fetchit7_content_field_fetcher($nid,'fetchit7_field_pre_function','value'));
	if (strlen($preqry)) {
		if (strtolower(substr($preqry,0,4)) == 'php.') {
			$thefunction = substr($preqry,4); 
			global $user;
    		$user_uid = (int) $user->uid;
			$result = $thefunction($user_uid);
		} else {
			$result = fetchit_run_query($db_type,$db_handle,$preqry);
		}
		//TODO: check pre function result?
	}

	// get database source data view/table name - this is required - can not be missing
	$view_name = trim(fetchit7_content_field_fetcher($nid,'source_view','value'));
	if(strlen($view_name)) {
			$form['view_name'] = array('#type' => 'value','#value' => trim($view_name));
	} else {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data view/table name.",'error');
		return $form;
	}

	// get table selection type - this is required - can not be missing
	$table_selection_type = trim(fetchit7_content_field_fetcher($nid,'table_selection_type','value'));
	if(strlen($table_selection_type)) {		//convert it to a number so it can work with old fetchit2 code...
		//convert it to a number so it can work with old fetchit2 code...
		//TODO: convert fetchit2 based code to use a string db_type
		if (!is_null($table_selection_type_number = array_search($table_selection_type, array(0 => 'none', 1 => 'copy_single', 2 => 'copy_multiple', 3 => 'delete_multiple')))) {
			$form['record_select'] = array('#type' => 'value','#value' => $table_selection_type_number);
		} else {
			drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the record selection type.",'error');
			return $form;
		}
	} else {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the record selection type.",'error');
		return $form;
	}

	// get table selection table - this is required for the copy selection types
	$table_selection_table = trim(fetchit7_content_field_fetcher($nid,'table_selection_table','value'));
	$form['record_select_table'] = array('#type' => 'value','#value' => $table_selection_table);
	if (in_array($table_selection_type,array('copy_single','copy_multiple'))) {
		if(!strlen($table_selection_table)) {
			drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the required record selection table name. A record selection table name is required when selecting copy single or copy multiple record selection types.",'error');
			return $form;
		}
	}

	// get drupal user id (uid) field name 
	$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
	$form['user_uid_field'] = array('#type' => 'value','#value' => $user_uid_field);

	// get key field name 
	$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
	$form['key_field'] = array('#type' => 'value','#value' => $key_field);

	// get submit button label 
	$submit_label = trim(fetchit7_content_field_fetcher($nid,'submit_label','value'));
	$form['submit_label'] = array('#type' => 'value','#value' => $submit_label);
	if (!strlen($submit_label)) $submit_label = 'Submit';
	
	// get the source table meta data
	$field_count = fetchit_get_field_count($db_type,$db_data_object);
	$form['field_count'] = array('#type' => 'value','#value' => $field_count);
	$row_count = fetchit_get_row_count($db_type,$db_data_object);
	$form['row_count'] = array('#type' => 'value','#value' => $row_count);
	$field_names = array();
	if (!($field_names = fetchit_get_field_names($db_type,$db_data_object))) {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field names.",'error');
		return $form;
	}
	$field_types = array();
	if (!($field_types = fetchit_get_field_types($db_type,$db_data_object))) {
		drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field types.",'error');
		return $form;
	}
	
	// existing record selections use the key field and a separate table containing selected key(s)
	// could be a single value or an array of values
	// skip implementing this for now ... until after testing is done
	//$existing_selections = fetchit_get_existing_selection_nid($db_handle,$db_type,$nid);
	
	// loop through the fields and determine if they are either a source data key field or a drupal user id field
	// these are not displayed in the table or tableselect element
	// the key field values will become the options[] array keys used in record selection,
	//   and the drupal user id field is used in queries.
	$uid_field_ndx = 0;
	$key_field_ndx = 0;
	for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) { // loop over the source table fields
		if ( $field_names[$field_ndx] == $user_uid_field ) {
			// save it as a 1-based field index but do not create an output table column
			$uid_field_ndx = $field_ndx + 1;
			$form['uid_field_ndx'] = array('#type' => 'value','#value' => $uid_field_ndx);
		}
		if ( $field_names[$field_ndx] == $key_field ) {
			// save it as a 1-based field index but do not create an output table column
			$key_field_ndx = $field_ndx + 1;
			$form['key_field_ndx'] = array('#type' => 'value','#value' => $key_field_ndx);
		}
	}
	
	// create the render arrays for each case
	switch ($table_selection_type) {
		case 'copy_single': // single row selection and key field copy via radio buttons
			break;
		case 'copy_multiple': // multiple row selection and key field copy via check boxes
			break;
		case 'delete_multiple': // multiple row selection and record deletion via check boxes
			break;
		case 'none': // simple html table display
		default:
					
			// test - see if we can get the source data table and output it
			$query = "SELECT * FROM $view_name";
			$data_object = pg_query($db_handle,$query);
			$field_count = pg_num_fields($data_object);
			$row_count = pg_num_rows($data_object);
			if ($field_count) {
				$field_names = array();
				for ($j = 0; $j < $field_count; $j++) {
					$field_names[] = pg_field_name($data_object, $j);
				}
			}
	
			// create the simple HTML table header and rows
			$header = array();
			$rows = array();
			for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
				$row = array();
				for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
					if ($row_ndx == 0) {
						$header[] = $field_names[$field_ndx];
					}
					$db_value = pg_fetch_result($data_object,$row_ndx,$field_ndx);
					$row[] = $db_value;
				}
				$rows[] = $row;
			}
	
			// create the HTML table render array
			$form['mytable'] = array(
				'#theme' => 'table',
				'#header' => $header,
				'#rows' => $rows,
				'#empty' => 'empty',
			);
	}
	// text element
	$form['fetchit7_table_text'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_table_text','fetchit7_table_text_default_value'),
		'#title' => t('Enter some text'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => $submit_label,
	);

	//$form['#validate'][] = 'fetchit7_table_form_validate';
	//$form['#submit'][] = 'fetchit7_table_form_submit';

	return $form;
	
}

/**
 * Process form validation
 */
function fetchit7_table_form_validate ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_table_form_validate");
}

/**
 * Process form submission
 */
function fetchit7_table_form_submit ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_table_form_submit");
	// reset the text
	if(!strlen(trim($string = $form_state['values']['fetchit7_table_text']))) $string = 'fetchit7_table_text_default_value';
	variable_set('fetchit7_table_text',$string);
	drupal_set_message(t('Successfully submitted changes.'),'status');
}


//===================================
// fetchit7 parameter input form content
//===================================

/**
  * Form builder.  Display fetchit7 parameter input content.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_param_form($form, &$form_state) {
	// form builder function

	// text element
	$myform['fetchit7_param_text'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_param_text','fetchit7_param_text_default_value'),
		'#title' => t('Enter some text'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$myform['#submit'][] = 'fetchit7_param_form_submit';
	return system_settings_form($myform);
	
}

/**
 * Process form validation
 */
function fetchit7_param_form_validate ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_param_form_validate");
}

/**
 * Process form submission
 */
function fetchit7_param_form_submit ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_param_form_submit");
	// reset the text
	if(!strlen(trim($string = $form_state['values']['fetchit7_param_text']))) $string = 'fetchit7_param_text_default_value';
	variable_set('fetchit7_param_text',$string);
}



//===================================
// testing functions...
//===================================

/**
  * Form builder.  Display fetchit7 content.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_xxx_form() {
	// form builder function

	// text element
	$form['fetchit7_xxx_text'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_xxx_text','xxx_default_value'),
		'#title' => t('Enter some text'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$form['#submit'][] = 'fetchit7_xxx_form_submit';
	return system_settings_form($form);
	
}

/**
 * Process form validation
 */
function fetchit7_xxx_form_validate ($form, $form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_xxx_form_validate");
}

/**
 * Process form submission
 */
function fetchit7_xxx_form_submit ($form, $form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_xxx_form_submit");
	// reset the text
	if(!strlen(trim($string = $form_state['values']['fetchit7_xxx_text']))) $string = 'xxx_default_value';
	variable_set('fetchit7_xxx_text',$string);
}


/**
 * node field name mapper
 * 
 * converts fetchit7 internal informal content field names into formal drupal node schema field names
 * these are the array key names in the $node array used in fetchit7_content_field_fetcher()
 * these are the basic drupal node table fields
 * 
 * this needs to carefully be kept in sync with the Drupal node schema definition when moving to Drupal 8!!!!!
 */

function fetchit7_node_field_mapper ($field_id) {
	$field_name = null;
	switch ($field_id) {
		// drupal basic node fields
		// TODO: put more basic drupal node fields in here
		case 'nid':
			$field_name = 'nid';
			break;
		case 'vid':
			$field_name = 'vid';
			break;
		case 'type':
		case 'node_type':
		case 'content_type':
			$field_name = 'type';
			break;
		case 'title':
		case 'node_title':
		case 'content_title':
			$field_name = 'title';
			break;
		default:
			// do ntohing, we want it to return null in that case
			break;
	}

	return $field_name;
}



/**
 * content field name mapper
 * 
 * converts fetchit7 internal informal content field names into formal drupal entity schema field names
 * these are the names that need to be entered into the drupal 7 field_get_items() used in fetchit7_content_field_fetcher()
 * also includes the basic drupal node fields in case those are useful, too
 * 
 * this needs to carefully be kept in sync with the schema definition in fetchit7.install!!!!!
 * 
 * TODO: fill out this switch statement with all the content fields
 */
function fetchit7_content_field_mapper ($field_id) {
	$field_name = null;
	switch ($field_id) {
		case 'body':
		case 'body_value':
			$field_name = 'body';
			break;
		// fetchit7 custom fields used by multiple content types
		case 'location':
		case 'fetchit7_field_location':
			$field_name = 'fetchit7_field_location';
			break;
		case 'enabled':
		case 'fetchit7_field_enabled':
			$field_name = 'fetchit7_field_enabled';
			break;
		case 'db_host':
		case 'fetchit7_field_db_host':
			$field_name = 'fetchit7_field_db_host';
			break;
		case 'db_type':
		case 'fetchit7_field_db_type':
			$field_name = 'fetchit7_field_db_type';
			break;
		case 'db_port':
		case 'fetchit7_field_db_port':
			$field_name = 'fetchit7_field_db_port';
			break;
		case 'db_name':
		case 'fetchit7_field_db_name':
			$field_name = 'fetchit7_field_db_name';
			break;
		case 'db_login':
		case 'fetchit7_field_db_login':
			$field_name = 'fetchit7_field_db_login';
			break;
		case 'db_password':
		case 'fetchit7_field_db_password':
			$field_name = 'fetchit7_field_db_password';
			break;
		case 'pre_function':
		case 'fetchit7_field_pre_function':
			$field_name = 'fetchit7_field_pre_function';
			break;
		case 'post_function':
		case 'fetchit7_field_post_function':
			$field_name = 'fetchit7_field_post_function';
			break;
		case 'view_name':
		case 'source_view':
		case 'fetchit7_field_source_view':
			$field_name = 'fetchit7_field_source_view';
			break;
		case 'uid_field':
		case 'user_uid_field':
		case 'fetchit7_field_uid_field':
			$field_name = 'fetchit7_field_uid_field';
			break;
		case 'key_field':
		case 'fetchit7_field_key_field':
			$field_name = 'fetchit7_field_key_field';
			break;
		case 'submit_label':
		case 'fetchit7_field_submit_label':
			$field_name = 'fetchit7_field_submit_label';
			break;
		// fetchit7 custom fields used by html table content type
		case 'record_select':
		case 'table_selection_type':
		case 'fetchit7_field_tbl_sel_typ':
			$field_name = 'fetchit7_field_tbl_sel_typ';
			break;
		case 'record_select_table':
		case 'table_selection_table':
		case 'fetchit7_field_tbl_sel_tbl':
			$field_name = 'fetchit7_field_tbl_sel_tbl';
			break;
		default:
			break;
	}
	return $field_name;
}


/**
 * content field fetcher
 * 
 * returns a content type field (or fields in some cases) in various formats for a given node id (nid)
 * the field id can be the official name or some local fetchit7 abbreviation,
 * as long as it is handled in the fetchit7 node field name mapper function, fetchit7_node_field_mapper()
 * or the fetchit7 content field name mapper function, fetchit7_content_field_mapper()
 * includes the basic node fields in case those are useful
 */
function fetchit7_content_field_fetcher ($nid, $field_id = 'title', $format = 'value') {
	$output = array();
	if ($node = node_load($nid)) {
		if ($field_name = fetchit7_node_field_mapper ($field_id)) {
			// it is a recognized node field so get the value from $node object
			//TODO: fetchit7_content_field_fetcher only outputs value for node fields, add the other output types (value will work for now, so this is a low priority)
			$output = $node->$field_name;
		} elseif ($field_name = fetchit7_content_field_mapper ($field_id)) {
			// assume it is a custom content field and use field_get_items() to get the value
			$entity_type = 'node';
			$items = field_get_items($entity_type,$node,$field_name);
			switch ($format) {
				case 'item':
					$output = $items[0];
					break;
				case 'items':
					$output = $items;
					break;
				case 'display':
					$output = field_view_value($entity_type,$node,$field_name,$items[0]);
					break;
				case 'value':
				default:
					$output = $items[0]['value'];
			}
		} else {
			// for now do nothing
		}
		return $output;
	}
}

/**
 * here are all the fetchit2 functions, tweaked as necessary, but still the same logic
 */
function fetchit_get_db_handle($nid) {
	$db_handle = null;
	if($db_type = fetchit7_content_field_fetcher($nid,'db_type','value')) {
		switch ($db_type) {
			case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
				// these are required for remote pg db connections
				if(!strlen(trim($db_host = fetchit7_content_field_fetcher($nid,'db_host','value')))) {
					return $db_handle;
				}
				if(!strlen(trim($db_port = fetchit7_content_field_fetcher($nid,'db_port','value')))) {
					return $db_handle;
				}
				if(!strlen(trim($db_name = fetchit7_content_field_fetcher($nid,'db_name','value')))) {
					return $db_handle;
				}
				if(!strlen(trim($db_login = fetchit7_content_field_fetcher($nid,'db_login','value')))) {
					return $db_handle;
				}
				if(!strlen(trim($db_password = fetchit7_content_field_fetcher($nid,'db_password','value')))) {
					return $db_handle;
				}
				// create the pg connection
				$db_string = "host=$db_host port=$db_port user=$db_login password=$db_password dbname=$db_name";
				$db_handle = pg_connect($db_string);
				if(!$db_handle) {
					return $db_handle;
				}
				break;
			case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
				break;
			case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
				break;
			default:
		}
	}
	return $db_handle;
}

function fetchit_run_query($db_type,&$db_handle,$query) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			$result = pg_query($db_handle,$query);
		case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
			break;
		case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
			break;
		default:
	}
	return $result;
}

function fetchit_get_field_count($db_type,&$data_object) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			if(!$field_count = pg_num_fields($data_object)) {
				return FALSE;
			}
			break;
		case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
			return FALSE;
			break;
		case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
			return FALSE;
			break;
		default:
			return FALSE;
	}
	return $field_count;
}

function fetchit_get_row_count($db_type,&$data_object) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			if(!$row_count = pg_num_rows($data_object)) {
				return FALSE;
			}
			break;
		case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
			return FALSE;
			break;
		case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
			return FALSE;
			break;
		default:
			return FALSE;
	}
	return $row_count;
}

function fetchit_get_field_names($db_type,&$data_object) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			if($field_count = pg_num_fields($data_object)) {
				$field_names = array();
				for ($j = 0; $j < $field_count; $j++) {
					$field_names[] = pg_field_name($data_object, $j);
				}
			} else {
				return FALSE;
			}
			break;
		case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
			return FALSE;
			break;
		case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
			return FALSE;
			break;
		default:
			return FALSE;
	}
	return $field_names;
}

function fetchit_get_field_types($db_type,&$data_object) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			if($field_count = pg_num_fields($data_object)) {
				$field_types = array();
				for ($j = 0; $j < $field_count; $j++) {
					$field_types[] = pg_field_type($data_object, $j);
				}
			} else {
				return FALSE;
			}
			break;
		case FETCHIT7_DBTYPE_NAME_MYSQL: //'mysql':
			return FALSE;
			break;
		case FETCHIT7_DBTYPE_NAME_MSSQL: //'sqlserver':
			return FALSE;
			break;
		default:
			return FALSE;
	}
	return $field_types;
}

