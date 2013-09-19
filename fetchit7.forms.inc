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
	$append_record_flag = FALSE;
	$parameter_select_flag = FALSE;
	$text_input_flag = FALSE;
	$table_weight = -8; // it goes at the top of the form
	$submit_weight = 8; // it goes at the bottom of the form
	
	// get nid embedded into original form_id; format fetchit7_table_form_X### - nid starts at 21
	$form_id = $form_state['build_info']['form_id'];
	$nid = (int) substr($form_id, 21);
	$form['nid'] = array('#type' => 'value','#value' => $nid);

	// get database type and store it (hidden) in the form so it is available to the form validate and submit functions
	$db_type = trim(fetchit7_content_field_fetcher($nid,'db_type','value'));
	if(strlen($db_type)) {
		$form['db_type'] = array('#type' => 'value','#value' => $db_type);
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
	if(strlen($table_selection_type)) {
		$form['record_select'] = array('#type' => 'value','#value' => $table_selection_type);
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

	// get table weight value 
	$table_weight = trim(fetchit7_content_field_fetcher($nid,'table_weight','value'));

	// get submit button label 
	$submit_label = trim(fetchit7_content_field_fetcher($nid,'submit_label','value'));
	$form['submit_label'] = array('#type' => 'value','#value' => $submit_label);
	if (strlen($submit_label)) {
		$submit_button_flag = TRUE;
	} else {
		switch ($table_selection_type) {
			case 'none':
				if ($text_input_flag) {
					$submit_button_flag = TRUE;
					$submit_label = 'Submit';
				} elseif ($parameter_select_flag) {
					$submit_button_flag = TRUE;
					$submit_label = 'Submit';
				} else {
					$submit_button_flag = FALSE;
				}
				break;
			case 'copy_single':
			case 'copy_multiple':
				$submit_button_flag = TRUE;
				$submit_label = 'Copy Records';
				break;
			case 'delete_multiple':
				$submit_button_flag = TRUE;
				$submit_label = 'Delete Records';
				break;
			default:
				$submit_button_flag = TRUE;
				$submit_label = 'Submit';
		}
	}
	// get table weight value 
	$submit_weight = trim(fetchit7_content_field_fetcher($nid,'submit_weight','value'));

	// get the source data into a data object
	if (!($db_data_object = fetchit_get_data_object($db_type,$db_handle,$view_name,$user_uid_field))) {
			drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when querying the source data table.",'error');
			return $form;
	}
			
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
		case 'copy_multiple': // multiple row selection and key field copy via check boxes
		case 'delete_multiple': // multiple row selection and record deletion via check boxes
			// create the HTML tableselect header and options
			$header = array();
			$options = array();
			for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
				$row = array();
				$options_key = null;
				$column_count = 0;
				for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
					$db_value = fetchit_get_data($db_type,$db_data_object,$row_ndx,$field_ndx);
					if ($field_ndx == ($uid_field_ndx-1) ) {
						// don't do anything with this value
					} else {
						if ($field_ndx == ($key_field_ndx-1)) {
							// use this as the key for the options array
							$options_key = $db_value;
							if ($row_ndx == 0) {
								$default_value = $db_value;
							}
						} else {
							if ($row_ndx == 0) {
								$header['column'.$column_count] = $field_names[$field_ndx];
							}
							$row['column'.$column_count] = $db_value;
							++$column_count;
						}
					}
				}
				if (is_null($options_key)) { // selection enabled but no key field given - shame on you, user!!
					$options_key = $row_ndx;
				}
				$options[$options_key] = $row;
			}
			if ($table_selection_type == 'copy_single') {
				$multiple_flag = FALSE;
				$selectall_flag = FALSE;
			} else {
				$multiple_flag = TRUE;
				$selectall_flag = TRUE;
				$default_value = array(); // reset to nothing selected
			}
			// create the HTML tableselect render array with a single select option
			$form['table'] = array(
					'#type' => 'tableselect',
					'#header' => $header,
					'#options' => $options,
					'#default_value' => $default_value,
					'#multiple' => $multiple_flag,
					'#js_select' => $selectall_flag,
					'#empty' => 'Nothing to see here.  Move along, please.',
					'#weight' => $table_weight,
			);
			break;

		case 'none': // simple html table display
		default:
			// create the simple HTML table header and rows
			$header = array();
			$rows = array();
			for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
				$row = array();
				for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
					if ($field_ndx != ($uid_field_ndx-1) ) {
						if ($field_ndx != ($key_field_ndx-1)) {
							if ($row_ndx == 0) {
								$header[] = $field_names[$field_ndx];
							}
							$db_value = fetchit_get_data($db_type,$db_data_object,$row_ndx,$field_ndx);
							$row[] = $db_value;
						}
					}
				}
				$rows[] = $row;
			}

			// create the HTML table render array
			$form['table'] = array(
				'#theme' => 'table',
				'#header' => $header,
				'#rows' => $rows,
				'#empty' => 'Nothing to see here.  Move along, please.',
				'#weight' => $table_weight,
			);
	}

	// append record elements
	if ($append_record_flag) {
		$form['fetchit7_table_text'] = array(
			'#type' => 'textfield',
			'#default_value' => variable_get('fetchit7_table_text','fetchit7_table_text_default_value'),
			'#title' => t('Enter some text'),
			'#size' => 30,
			'#maxlength' => 60,
			'#required' => TRUE,
		);
	}

	// parameter select element
	if ($parameter_select_flag) {
		$form['fetchit7_table_text'] = array(
			'#type' => 'textfield',
			'#default_value' => variable_get('fetchit7_table_text','fetchit7_table_text_default_value'),
			'#title' => t('Enter some text'),
			'#size' => 30,
			'#maxlength' => 60,
			'#required' => TRUE,
		);
	}

	// text input element
	if ($text_input_flag) {
		$form['fetchit7_table_text'] = array(
			'#type' => 'textfield',
			'#default_value' => variable_get('fetchit7_table_text','fetchit7_table_text_default_value'),
			'#title' => t('Enter some text'),
			'#size' => 30,
			'#maxlength' => 60,
			'#required' => TRUE,
		);
	}
	
	if ($submit_button_flag) {
		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => $submit_label,
			'#weight' => $submit_weight,
		);
	}

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
	// get the form nid 
	$nid = $form['nid']['#value'];
	// get the table type
	$record_select = $form['record_select']['#value'];
	// check out the table for errors
	switch ($record_select) {
		case 0:
			// nothing really to check on a plain html table
			break;
		case 1: //radio buttons
			$selected = $form_state['values']['table'];
			if(is_null($selected)) {
				form_set_error('submit',t("Invalid selection: $selected  A selection is required before pressing the submit button."));
				return;
			}
			$key_field = $form['key_field']['#value'];
			if (!strlen($key_field)) {
				form_set_error('submit',t('fetchit7 setup error: missing key field name.  A key field name is required.'));
				return;
			}
			$record_select_table = $form['record_select_table']['#value'];
			if (!strlen($record_select_table)) {
				form_set_error('submit',t('fetchit7 setup error: missing selection table name.  A selection table name is required.'));
				return;
			}
			break;
		case 2: //checkboxes for record selection
			$key_field = $form['key_field']['#value'];
			if (!strlen($key_field)) {
				form_set_error('submit',t('fetchit7 setup error: missing key field name.  A key field name is required.'));
				return;
			}
			$record_select_table = $form['record_select_table']['#value'];
			if (!strlen($record_select_table)) {
				form_set_error('submit',t('fetchit setup error: missing selection table name.  please contact the site administrator.'));
				return;
			}
			break;
		case 3: //checkboxes for record deletion
			$key_field = $form['key_field']['#value'];
			if (!strlen($key_field)) {
				form_set_error('submit',t('fetchit7 setup error: missing key field name.  A key field name is required.'));
				return;
			}
			break;
		default:
	}
}

/**
 * Process form submission
 */
function fetchit7_table_form_submit ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_table_form_submit");
	
	$nid = $form['nid']['#value'];
	$record_select = $form['record_select']['#value'];
	$db_type = $form['db_type']['#value'];
	$db_handle = $form['db_handle']['#value'];
	switch ($record_select) {
		case 'none': // static html - no selection - do nothing
			break;
		case 'copy_single':  // there is always one value when it is working correctly
			$key_field_ndx = $form['key_field_ndx']['#value'];
			if ($key_field_ndx) {
				$db_type = $form['db_type']['#value'];
				$db_handle = $form['db_handle']['#value'];
				$selected_radio_option = $form_state['values']['table'];
				if (is_null($selected_radio_option)) {
					drupal_set_message(t("Selection not saved because no records were selected."),'warning');
				} else {
					// now the D7 tableselect actually returns the key field value
					$key_field_val = $selected_radio_option;
					if ($result = fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$key_field_val)) {
						drupal_set_message(t("Selection saved."),'status');
					} else {
						drupal_set_message(t("The selected record was NOT saved. Please contact the site administrator."),'warning');
					}
				}
			} else {
				drupal_set_message(t("The selected record was NOT saved because a table key field was not detected.  Please contact the site administrator."),'warning');
			}
			break;
		case 'copy_multiple':
			$key_field_ndx = $form['key_field_ndx']['#value'];
			if ($key_field_ndx) {
				$db_type = $form['db_type']['#value'];
				$db_handle = $form['db_handle']['#value'];
				$selected_checkbox_options = array_filter($form_state['values']['table']);
				// now the D7 tableselect returns the key field value
				$key_field_vals = $selected_checkbox_options;
				if (array_count_values($key_field_vals)) {
					$result = fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$key_field_vals);
					if ($result) {
						drupal_set_message(t("Selections ($result records) saved."),'status');
					} else {
						drupal_set_message(t("The selected records were NOT saved. Please contact the site administrator."),'warning');
					}
				} else {
					$result = fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$key_field_vals);
					drupal_set_message(t("Selections ($result records) saved."),'status');
				}
			} else {
				drupal_set_message(t("The selected records were NOT saved because a table key field was not detected.  Please contact the site administrator."),'warning');
			}
			break;
		case 'delete_multiple':
			$key_field_ndx = $form['key_field_ndx']['#value'];
			if ($key_field_ndx) {
				$db_type = $form['db_type']['#value'];
				$db_handle = $form['db_handle']['#value'];
				$selected_checkbox_options = array_filter($form_state['values']['table']);
				$key_field_vals = $selected_checkbox_options;
				if (array_count_values($key_field_vals)) {
					$result = fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$key_field_vals);
					if ($result) {
						drupal_set_message(t("Selections ($result records) deleted."),'status');
					} else {
						drupal_set_message(t("The selected records were NOT deleted. Please contact the site administrator."),'warning');
					}
				} else {
					$result = fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$key_field_vals);
					drupal_set_message(t("Selections ($result records) deleted."),'status');
				}
			} else {
				drupal_set_message(t("The selected records were NOT deleted because a table key field was not detected.  Please contact the site administrator."),'warning');
			}
			break;
		default:
	}

	//run the post query
	$postqry = trim(fetchit7_content_field_fetcher($nid,'fetchit7_field_post_function','value'));
	if (strlen($postqry)) {
		if (strtolower(substr($postqry,0,4)) == 'php.') {
			$thefunction = substr($postqry,4); 
			global $user;
    		$user_uid = (int) $user->uid;
			$result = $thefunction($user_uid);
		} else {
			$result = fetchit_run_query($db_type,$db_handle,$postqry);
		}
	}

	// redirect to the next page
	$redirect = trim(fetchit7_content_field_fetcher($nid,'fetchit7_field_submit_redirect','value'));
	if (strlen($redirect)) {
		$form_state['redirect'] = $redirect; 
	}
	
	// reset the text
	//drupal_set_message(t('Successfully submitted changes.'),'status');
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
	$table_weight = -8; // it goes at the top of the form
	$submit_weight = 8; // it goes at the bottom of the form
	
	// get nid embedded into original form_id; format fetchit7_param_form_X### - nid starts at 21
	$form_id = $form_state['build_info']['form_id'];
	$nid = (int) substr($form_id, 21);
	$form['nid'] = array('#type' => 'value','#value' => $nid);

	// get database type and store it (hidden) in the form so it is available to the form validate and submit functions
	$db_type = trim(fetchit7_content_field_fetcher($nid,'db_type','value'));
	if(strlen($db_type)) {
		$form['db_type'] = array('#type' => 'value','#value' => $db_type);
	} else {
		drupal_set_message("An error occurred in fetchit7_param_form (nid = $nid) when accessing the database type.",'error');
		return $form;
	}

	// get database handle
	if ($db_handle = fetchit_get_db_handle($nid)) {
		$form['db_handle'] = array('#type' => 'value','#value' => $db_handle);
	} else {
		drupal_set_message("An error occurred in fetchit7_param_form (nid = $nid) when creating the database connection handle.",'error');
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

	
	// submit button attributes
	$submit_label = trim(fetchit7_content_field_fetcher($nid,'fetchit7_field_submit_label','value'));
	//$form['submit_label'] = array('#type' => 'value','#value' => $submit_label);
	if (!(strlen($submit_label))) {
		$submit_label = 'Submit';
	}
	$submit_weight = fetchit7_content_field_fetcher($nid,'fetchit7_field_submit_weight','value');
	$submit_redirect = fetchit7_content_field_fetcher($nid,'fetchit7_field_submit_redirect','value');
	// table attributes 
	$showtitles = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_shw_ttl','value');
	$table_weight = fetchit7_content_field_fetcher($nid,'fetchit7_field_table_weight','value');
	$table_rows = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_tbl_rows','value');
	//these should all be the same sized arrays or something is whacked
	$param_names = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_name','values'); //should return an array
	$param_titles = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_title','values'); //should return an array
	$param_descriptions = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_desc','values'); //should return an array
	$param_types = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_type','values'); //should return an array
	$param_enabled = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_enabled','values'); //should return an array
	$param_required = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_required','values'); //should return an array
	$param_str = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_parstr','values'); //should return an array

	$param_list_tbl = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_list_tbl','values'); //should return an array
	$param_list_key = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_list_key','values'); //should return an array
	$param_list_dsp = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_list_dsp','values'); //should return an array
	$param_list_uid = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_list_uid','values'); //should return an array
	$param_list_def = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_list_def','values'); //should return an array

	$param_data_tbl = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_tbl','values'); //should return an array
	$param_data_key = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_key','values'); //should return an array
	$param_data_uid = fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_uid','values'); //should return an array

	//use the above arrays to contruct the $user_field_data array allowing the use of the fetchit2 code AMAP
	// note that param names DEFINES the count of the fields
	// we will use defaults is possible where items are missing
	// exit gracefully with error message otherwise
	$control_list = array( // add 1 to make array_search() easier.  subtract 1 later...
		F_DR_CONTROL_TEXTBOX + 1 => FETCHIT7_CONTROLTYPE_NAME_TEXTBOX,
		F_DR_CONTROL_DATESEL + 1 => FETCHIT7_CONTROLTYPE_NAME_DATESEL,
		F_DR_CONTROL_DROPDOWN_SINGLE + 1 => FETCHIT7_CONTROLTYPE_NAME_DROPDOWN_SINGLE,
		F_DR_CONTROL_RADIOBTN + 1 => FETCHIT7_CONTROLTYPE_NAME_RADIOBTN,
		F_DR_CONTROL_CHECKBOXES + 1 => FETCHIT7_CONTROLTYPE_NAME_CHECKBOXES,
		F_DR_CONTROL_DROPDOWN_MULTI + 1 => FETCHIT7_CONTROLTYPE_NAME_DROPDOWN_MULTI,
		F_DR_CONTROL_CHECKBOX + 1 => FETCHIT7_CONTROLTYPE_NAME_CHECKBOX,
		F_DR_CONTROL_DATETIMESEL + 1 => FETCHIT7_CONTROLTYPE_NAME_DATETIMESEL,
		F_DR_CONTROL_POPUPDATE + 1 => FETCHIT7_CONTROLTYPE_NAME_POPUPDATE,
	);
	$control_list_string = '';
	$count = 0;
	foreach ($control_list as $control_string) {
		if ($count) $control_list_string .= ', ';
		$control_list_string .= $control_string;
		++$count;
	}
	$user_field_data = array();
	$param_count = array_count($param_names);
	$field_index = 0; // the actually created parameter fields
	$param_index = 0; // the list of parameter values created by the user - just in case the returned array does not have 0 based integer keys
	// even though the fetchit2 does this too...do some data preprocessing 
	foreach ($param_names as $key => $param_name) {
		if (strlen($param_name=trim($param_name))) { // skip it if blank
			$user_field_data[$field_index][F_DR_FIELD_NAME_STR] = $param_name;
			//title default of '' is OK
			if ($param_index < array_count($param_titles)) {
				$user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = trim($param_titles[$param_index]);
			} else {
				$user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = '';
			}
			//description default of '' is OK
			if ($param_index < array_count($param_descriptions)) {
				$user_field_data[$field_index][F_DR_FIELD_DESC_STR] = trim($param_descriptions[$param_index]);
			} else {
				$user_field_data[$field_index][F_DR_FIELD_DESC_STR] = '';
			}
			// types require some more checking since they are typed into a text box
			if ($param_index < array_count($param_types)) {
				// see if the control type matches something we know about
				if ($key = array_search(strtolower(trim($param_types[$param_index])), $control_list)) {
					$user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = $key-1; // the fetchit2 code wants an index here, and we had incremented these by 1 above to make array_search() easier to use
				} else {
					drupal_set_message("Parameter $param_index control type (".trim($param_types[$param_index]).") unrecognized.  Valid control types are [$control_list_string]. Please contact the site administrator.",'error');
					return;
				}
			} else {
				// can't have a default here, too many complications
				drupal_set_message("Parameter $param_index control type missing.  There must be as many control types specified as there are parameter names. Please contact the site administrator.",'error');
				return;
			}
			//$param_enabled
			//fetchit2 uses disabled - so default of disabled = true is OK
			if ($param_index < array_count($param_enabled)) {
				$enabled = filter_var(trim($param_enabled[$param_index]), FILTER_VALIDATE_BOOLEAN);
				$user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = !$enabled;
			} else {
				$user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = TRUE;
			}
						
			//$param_required
			//default of required = true is OK
			if ($param_index < array_count($param_required)) {
				$user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = filter_var(trim($param_required[$param_index]), FILTER_VALIDATE_BOOLEAN);
			} else {
				$user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = TRUE;
			}
						
			//parameter string ?what is this for?
			
			++$field_index;
		}
		++$param_index;
	}

	//$user_field_data = fetchit_get_user_field_data($nid);
	
	// this form will be a table of pre-rendered controls - hope it works!
		
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
		case 'table_weight':
		case 'fetchit7_field_table_weight':
			$field_name = 'fetchit7_field_table_weight';
			break;
		case 'submit_label':
		case 'fetchit7_field_submit_label':
			$field_name = 'fetchit7_field_submit_label';
			break;
		case 'submit_weight':
		case 'fetchit7_field_submit_weight':
			$field_name = 'fetchit7_field_submit_weight';
			break;
		case 'redirect':
		case 'submit_redirect':
		case 'fetchit7_field_submit_redirect':
			$field_name = 'fetchit7_field_submit_redirect';
			break;
		// fetchit7 custom fields used by html table content type
		case 'tbl_sel_typ':
		case 'record_select':
		case 'table_selection_type':
		case 'fetchit7_field_tbl_sel_typ':
			$field_name = 'fetchit7_field_tbl_sel_typ';
			break;
		case 'tbl_sel_tbl':
		case 'record_select_table':
		case 'table_selection_table':
		case 'fetchit7_field_tbl_sel_tbl':
			$field_name = 'fetchit7_field_tbl_sel_tbl';
			break;
		// fetchit7 custom fields used by parameter input content type
		case 'par_shw_ttl':
		case 'fetchit7_field_par_shw_ttl':
			$field_name = 'fetchit7_field_par_shw_ttl';
			break;
		case 'param_table_rows':
		case 'parameter_table_rows':
		case 'par_tbl_rows':
		case 'fetchit7_field_par_tbl_rows':
			$field_name = 'fetchit7_field_par_tbl_rows';
			break;
		// fetchit7_field_par_fld_name title desc type
		case 'par_fld_name':
		case 'fetchit7_field_par_fld_name':
			$field_name = 'fetchit7_field_par_fld_name';
			break;
		case 'par_fld_title':
		case 'fetchit7_field_par_fld_title':
			$field_name = 'fetchit7_field_par_fld_title';
			break;
		case 'par_fld_desc':
		case 'fetchit7_field_par_fld_desc':
			$field_name = 'fetchit7_field_par_fld_desc';
			break;
		case 'par_fld_type':
		case 'fetchit7_field_par_fld_type':
			$field_name = 'fetchit7_field_par_fld_type';
			break;
		case 'par_fld_enabled':
		case 'fetchit7_field_par_fld_enabled':
			$field_name = 'fetchit7_field_par_fld_enabled';
			break;
		case 'par_fld_required':
		case 'fetchit7_field_par_fld_required':
			$field_name = 'fetchit7_field_par_fld_required';
			break;
		case 'par_fld_parstr':
		case 'fetchit7_field_par_fld_parstr':
			$field_name = 'fetchit7_field_par_fld_parstr';
			break;
		case 'par_fld_list_tbl':
		case 'fetchit7_field_par_fld_list_tbl':
			$field_name = 'fetchit7_field_par_fld_list_tbl';
			break;
		case 'par_fld_list_key':
		case 'fetchit7_field_par_fld_list_key':
			$field_name = 'fetchit7_field_par_fld_list_key';
			break;
		case 'par_fld_list_dsp':
		case 'fetchit7_field_par_fld_list_dsp':
			$field_name = 'fetchit7_field_par_fld_list_dsp';
			break;
		case 'par_fld_list_uid':
		case 'fetchit7_field_par_fld_list_uid':
			$field_name = 'fetchit7_field_par_fld_list_uid';
			break;
		case 'par_fld_list_def':
		case 'fetchit7_field_par_fld_list_def':
			$field_name = 'fetchit7_field_par_fld_list_def';
			break;
		case 'par_fld_list_uid':
		case 'fetchit7_field_par_fld_list_uid':
			$field_name = 'fetchit7_field_par_fld_list_uid';
			break;
		case 'par_fld_data_tbl':
		case 'fetchit7_field_par_fld_data_tbl':
			$field_name = 'fetchit7_field_par_fld_data_tbl';
			break;
		case 'par_fld_data_key':
		case 'fetchit7_field_par_fld_data_key':
			$field_name = 'fetchit7_field_par_fld_data_key';
			break;
		case 'par_fld_data_uid':
		case 'fetchit7_field_par_fld_data_uid':
			$field_name = 'fetchit7_field_par_fld_data_uid';
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
	//$output = array();
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
				case 'values':
					$output = array();
					if (is_array($items)) {
						$keys = array_keys($items);
						foreach ($keys as $key) {
							$output[] = $items[$key]['value'];
						}
					}
					break;
				case 'value':
				default:
					$output = $items[0]['value'];
			}
		} else {
			// for now do nothing
			debug($nid,'nid');
			debug($field_id,'field_id');
			debug($format,'format');
			$output = array();
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

function fetchit_run_query($db_type,$db_handle,$query) {
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

function fetchit_get_data_object($db_type,&$db_handle,$view_name,$user_uid_field) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			$query = "SELECT * FROM $view_name";
			if (strlen($user_uid_field)) {
				global $user;
				$user_uid = $user->uid;
				$query .= " WHERE $user_uid_field = ".$user_uid;
			}
			$data_object = pg_query($db_handle,$query);
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
	return $data_object;
}

function fetchit_get_data($db_type,&$data_object,$row,$field) {
	switch ($db_type) {
		case FETCHIT7_DBTYPE_NAME_PGSQL: //'postgresql':
			$data = pg_fetch_result($data_object,$row,$field);
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
	return $data;
}

function fetchit_get_existing_selection_nid ($db_handle,$db_type,$nid) {
	if ($db_handle) {
		if ($db_type) {
			if ($nid) {
				$table_selection_type = trim(fetchit7_content_field_fetcher($nid,'table_selection_type','value'));
				switch ($table_selection_type) {
					case 'none': // static html - no selection - do nothing
						break;

					case 'copy_single': // radio button record selection
						$table_selection_table = trim(fetchit7_content_field_fetcher($nid,'table_selection_table','value'));
						$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
						$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
						$result = fetchit_get_existing_selection_single($db_handle,$db_type,$table_selection_table,$key_field,$user_uid_field); 
						return $result;
						break;

					case 'copy_multiple': // check boxes button record selection
						$table_selection_table = trim(fetchit7_content_field_fetcher($nid,'table_selection_table','value'));
						$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
						$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
						$result = fetchit_get_existing_selection_multiple($db_handle,$db_type,$table_selection_table,$key_field,$user_uid_field); 
						return $result;
						break;

					case 'delete_multiple':
					default:
				}
			}
		}
	}
}

function fetchit_set_existing_selection_nid ($db_handle,$db_type,$nid,$val) {
	if ($db_handle) {
		if ($db_type) {
			if ($nid) {
				$table_selection_type = trim(fetchit7_content_field_fetcher($nid,'table_selection_type','value'));
				switch ($table_selection_type) {
					case 'none': // static html - no selection - do nothing
						break;

					case 'copy_single': // radio button record selection
						$table_selection_table = trim(fetchit7_content_field_fetcher($nid,'table_selection_table','value'));
						$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
						$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
						$result = fetchit_set_existing_selection_single($db_handle,$db_type,$table_selection_table,$key_field,$user_uid_field,$val); 
						return $result;
						break;

					case 'copy_multiple': // check boxes button record selection
						$table_selection_table = trim(fetchit7_content_field_fetcher($nid,'table_selection_table','value'));
						$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
						$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
						$result = fetchit_set_existing_selection_multiple($db_handle,$db_type,$table_selection_table,$key_field,$user_uid_field,$val); 
						return $result;
						break;

					case 'delete_multiple':
						$deletion_table_name = trim(fetchit7_content_field_fetcher($nid,'source_view','value'));
						$user_uid_field = trim(fetchit7_content_field_fetcher($nid,'uid_field','value'));
						$key_field = trim(fetchit7_content_field_fetcher($nid,'key_field','value'));
						$result = fetchit_del_existing_selection_multiple($db_handle,$db_type,$deletion_table_name,$key_field,$user_uid_field,$val); 
						return $result;
						break;

					default:
				}
			}
		}
	}
}

function fetchit_get_existing_selection_single($db_handle,$db_type,$record_select_table,$key_field,$user_uid_field) {
	if($db_handle) {
		if($db_type) {
			switch ($db_type) {
				case FETCHIT7_DBTYPE_NAME_PGSQL:
					if(strlen(trim($record_select_table))) {
						if(strlen(trim($key_field))) {
							$qry = "SELECT $key_field FROM $record_select_table";
							if (strlen(trim($user_uid_field))) {
								global $user;
								$user_uid = $user->uid;
								$qry .= " WHERE $user_uid_field = $user_uid";
							}
							$qry .= " LIMIT 1";
							$db_object = pg_query($db_handle,$qry);
							$row = pg_fetch_object($db_object);
							$selection = $row->$key_field;
							return $selection;
						}
					}
					break;

				case FETCHIT7_DBTYPE_NAME_MYSQL:
				case FETCHIT7_DBTYPE_NAME_MSSQL:
				default:
			}
		}
	}
}

function fetchit_set_existing_selection_single($db_handle,$db_type,$record_select_table,$key_field,$user_uid_field,$val) {
	$insertedrecordcount = 0;
	if($db_handle) {
		if($db_type) {
			switch ($db_type) {
				case FETCHIT7_DBTYPE_NAME_PGSQL:
					if(strlen(trim($record_select_table))) {
						if(strlen(trim($key_field))) {
							$qry = "DELETE FROM $record_select_table";
							if (strlen(trim($user_uid_field))) {
								global $user;
								$qry .= " WHERE $user_uid_field = ".$user->uid;
							}
							$db_result = pg_query($db_handle,$qry);
							$fldstr = $key_field;
							if (strlen(trim($user_uid_field))) {
								$fldstr .= ",$user_uid_field";
							}
							
							$qry = "INSERT INTO $record_select_table($fldstr) VALUES(".pg_escape_string($val);
							if (strlen(trim($user_uid_field))) {
								global $user;
								$user_uid = $user->uid;
								$qry .= ",$user_uid";
							}
							$qry .= ")";
							if ($db_result = pg_query($db_handle,$qry)) {
								++$insertedrecordcount;
							}
						}
					}
					break;

				case FETCHIT7_DBTYPE_NAME_MYSQL:
				case FETCHIT7_DBTYPE_NAME_MSSQL:
				default:
			}
		}
	}
	return $insertedrecordcount;
}

function fetchit_get_existing_selection_multiple($db_handle,$db_type,$record_select_table,$key_field,$user_uid_field) {
	if($db_handle) {
		if($db_type) {
			switch ($db_type) {
				case FETCHIT7_DBTYPE_NAME_PGSQL:
					if(strlen(trim($record_select_table))) {
						if(strlen(trim($key_field))) {
							$qry = "SELECT $key_field FROM $record_select_table";
							if (strlen(trim($user_uid_field))) {
								global $user;
								$user_uid = $user->uid;
								$qry .= " WHERE $user_uid_field = $user_uid";
							}
							$selection = array();
							$db_object = pg_query($db_handle,$qry);
							while($row = pg_fetch_object($db_object)) {
								$selection[] = $row->$key_field;  
							};
							return $selection;
						}
					}
					break;

				case FETCHIT7_DBTYPE_NAME_MYSQL:
				case FETCHIT7_DBTYPE_NAME_MSSQL:
				default:
			}
		}
	}	
}

function fetchit_set_existing_selection_multiple($db_handle,$db_type,$record_select_table,$key_field,$user_uid_field,$vals) {
	$insertedrecordcount = 0;
	if($db_handle) {
		if($db_type) {
			switch ($db_type) {
				case FETCHIT7_DBTYPE_NAME_PGSQL:
					if(strlen(trim($record_select_table))) {
						if(strlen(trim($key_field))) {
							$qry = "DELETE FROM $record_select_table";
							if (strlen(trim($user_uid_field))) {
								global $user;
								$qry .= " WHERE $user_uid_field = ".$user->uid;
							}
							$db_object = pg_query($db_handle,$qry);
							$fldstr = $key_field;
							if (strlen(trim($user_uid_field))) {
								$fldstr .= ",$user_uid_field";
							}
							foreach ($vals as $val) {
								$qry = "INSERT INTO $record_select_table($fldstr) VALUES(".pg_escape_string($val);
								if (strlen(trim($user_uid_field))) {
									global $user;
									$user_uid = $user->uid;
									$qry .= ",$user_uid";
								}
								$qry .= ")";
								if($db_result = pg_query($db_handle,$qry)){
									++$insertedrecordcount;
								}
							}
						}
					}
					break;

				case FETCHIT7_DBTYPE_NAME_MYSQL:
				case FETCHIT7_DBTYPE_NAME_MSSQL:
				default:
			}
		}
	}
	return $insertedrecordcount;
}

function fetchit_del_existing_selection_multiple($db_handle,$db_type,$deletion_table,$key_field,$user_uid_field,$vals) {
	$deletedrecordcount = 0;
	if($db_handle) {
		if($db_type) {
			switch ($db_type) {
				case FETCHIT7_DBTYPE_NAME_PGSQL:
					if(strlen(trim($deletion_table))) {
						if(strlen(trim($key_field))) {
							foreach ($vals as $val) {
								$qry = "DELETE FROM $deletion_table WHERE $key_field = $val";
								if (strlen(trim($user_uid_field))) {
									global $user;
									$user_uid = $user->uid;
									$qry .= " AND $user_uid_field = $user_uid";
								}
								if($db_result = pg_query($db_handle,$qry)){
									++$deletedrecordcount;
								}
							}
						}
					}
					break;

				case FETCHIT7_DBTYPE_NAME_MYSQL:
				case FETCHIT7_DBTYPE_NAME_MSSQL:
				default:
			}
		}
	}	
	return $deletedrecordcount;
}

