<?php

/**
 * @file
 * Forms page callbacks for the fetchit7 module
 */

include_once (drupal_get_path('module', 'fetchit7') . '/fetchit7.constants.inc');
//include_once(drupal_get_path('module', 'fetchit7').'/fetchit2.constants.inc');

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
  $addbody_flag = TRUE; // until we have a flag for this it on the fetchit7 form
  $table_weight = -8;
  // it goes at the top of the form
  $submit_weight = 8;
  // it goes at the bottom of the form

  // get nid embedded into original form_id; format fetchit7_table_form_X### - nid starts at 21
  $form_id = $form_state['build_info']['form_id'];
  $nid = (int) substr($form_id, 21);
  $form['nid'] = array('#type' => 'value', '#value' => $nid);

  // get database type and store it (hidden) in the form so it is available to the form validate and submit functions
  $db_type = trim(fetchit7_content_field_fetcher($nid, 'db_type', 'value'));
  if (strlen($db_type)) {
    $form['db_type'] = array('#type' => 'value', '#value' => $db_type);
  }
  else {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the database type.", 'error');
    return $form;
  }

  // get database handle
  if ($db_handle = fetchit_get_db_handle($nid)) {
    $form['db_handle'] = array('#type' => 'value', '#value' => $db_handle);
  }
  else {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when creating the database connection handle.", 'error');
    return $form;
  }

  // run the pre query before doing anything else
  // TODO: figure out why pre-fn is getting hit twice prior to the form drawing!!!
  $preqry = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_pre_function', 'value'));
  if (strlen($preqry)) {
    if (strtolower(substr($preqry, 0, 4)) == 'php.') {
      $thefunction = substr($preqry, 4);
      global $user;
      $user_uid = (int)$user -> uid;
      $result = $thefunction($user_uid);
    }
    else {
      $result = fetchit_run_query($db_type, $db_handle, $preqry);
    }
    //TODO: check pre function result?
  }

  // get database source data view/table name - this is required - can not be missing
  $view_name = trim(fetchit7_content_field_fetcher($nid, 'source_view', 'value'));
  if (strlen($view_name)) {
    $form['view_name'] = array('#type' => 'value', '#value' => trim($view_name));
  }
  else {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data view/table name.", 'error');
    return $form;
  }

  // get table selection type - this is required - can not be missing
  $table_selection_type = trim(fetchit7_content_field_fetcher($nid, 'table_selection_type', 'value'));
  if (strlen($table_selection_type)) {
    $form['record_select'] = array('#type' => 'value', '#value' => $table_selection_type);
  }
  else {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the record selection type.", 'error');
    return $form;
  }

  // get table selection table - this is required for the copy selection types
  $table_selection_table = trim(fetchit7_content_field_fetcher($nid, 'table_selection_table', 'value'));
  $table_selection_default_flag = fetchit7_get_special_char_default($table_selection_table);
  $table_selection_table = fetchit7_strip_special_char_default($table_selection_table);
  //debug($table_selection_table,'before $table_selection_table');
  //debug($table_selection_default_flag,'before $table_selection_default_flag');
  if($table_selection_default_flag == '') {
    $table_selection_default_flag = '-';
  }
  //debug($table_selection_table,'after $table_selection_table');
  //debug($table_selection_default_flag,'after $table_selection_default_flag');
  $form['record_select_table'] = array('#type' => 'value', '#value' => $table_selection_table);
  if (in_array($table_selection_type, array('copy_single', 'copy_multiple'))) {
    if (!strlen($table_selection_table)) {
      drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the required record selection table name. A record selection table name is required when selecting copy single or copy multiple record selection types.", 'error');
      return $form;
    }
  }

  // get drupal user id (uid) field name
  $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
  $form['user_uid_field'] = array('#type' => 'value', '#value' => $user_uid_field);
	//debug($user_uid_field,'$user_uid_field');
  // get key field name
  $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
  $form['key_field'] = array('#type' => 'value', '#value' => $key_field);
	//debug($key_field,'$key_field');

  // get table weight value
  $table_weight = trim(fetchit7_content_field_fetcher($nid, 'table_weight', 'value'));

  // HTML table_headers should be an array of text values
  // warning: d7 does not return empty strings at all
  // if someone put spaces as a placeholder, trim them to an empty string
  $table_headers = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_headers', 'values'));
  //should return an array
  $form['table_headers'] = array('#type' => 'value', '#value' => $table_headers);
  $table_headers_count = count($table_headers);

  // HTML empty table message - a text string
  // if someone put spaces as a placeholder, trim them to an empty string
  $table_empty_message = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_emptymsg', 'value'));
  //should return a string
  $form['table_empty_message'] = array('#type' => 'value', '#value' => $table_empty_message);

  // addbody
  // addbody_flag should be a single boolean value
  // use it if something valid is entered.   if not set it to a default of true
  //$addbody_flag = fetchit7_content_field_fetcher($nid, 'addbody_flag', 'value');
  $form['addbody_flag'] = array('#type' => 'value', '#value' => $addbody_flag);

  // get submit button label
  $submit_label = trim(fetchit7_content_field_fetcher($nid, 'submit_label', 'value'));
  $form['submit_label'] = array('#type' => 'value', '#value' => $submit_label);
  if (strlen($submit_label)) {
    $submit_button_flag = TRUE;
  }
  else {
    switch ($table_selection_type) {
      case 'none':
        if ($text_input_flag) {
          $submit_button_flag = TRUE;
          $submit_label = 'Submit';
        }
        elseif ($parameter_select_flag) {
          $submit_button_flag = TRUE;
          $submit_label = 'Submit';
        }
        else {
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
  $submit_weight = trim(fetchit7_content_field_fetcher($nid, 'submit_weight', 'value'));

  // get the source data into a data object
  if (!($db_data_object = fetchit_get_data_object($db_type, $db_handle, $view_name, $user_uid_field))) {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when querying the source data table.", 'error');
    return $form;
  }

  // get the source table meta data
  $field_count = fetchit_get_field_count($db_type, $db_data_object);
  $form['field_count'] = array('#type' => 'value', '#value' => $field_count);
  $row_count = fetchit_get_row_count($db_type, $db_data_object);
  $form['row_count'] = array('#type' => 'value', '#value' => $row_count);
  $field_names = array();
  if (!($field_names = fetchit_get_field_names($db_type, $db_data_object))) {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field names.", 'error');
    return $form;
  }
  $field_types = array();
  if (!($field_types = fetchit_get_field_types($db_type, $db_data_object))) {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field types.", 'error');
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
  for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {// loop over the source table fields
    if ($field_names[$field_ndx] == $user_uid_field) {
      // save it as a 1-based field index but do not create an output table column
      $uid_field_ndx = $field_ndx + 1;
      $form['uid_field_ndx'] = array('#type' => 'value', '#value' => $uid_field_ndx);
    }
		//debug($field_ndx,'$field_ndx');
		//debug($field_names[$field_ndx],'$field_names[$field_ndx]');
		//debug($key_field,'$key_field');
    if ($field_names[$field_ndx] == $key_field) {
      // save it as a 1-based field index but do not create an output table column
      $key_field_ndx = $field_ndx + 1;
      $form['key_field_ndx'] = array('#type' => 'value', '#value' => $key_field_ndx);
    }
  }
	//debug($uid_field_ndx,'$uid_field_ndx');
	//debug($key_field_ndx,'$key_field_ndx');
  // create the data for no selection
  $none_selection = null;
  $none_selections = array();
  // create the data for previous selection
  $previous_selection = fetchit_get_existing_selection_single($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field);
  $selections = fetchit_get_existing_selection_multiple($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field);
  $previous_selections = array();
  foreach ($selections as $selection) {
     $previous_selections[$selection] = TRUE; 
  }
  // initialize the data for all and first selection
  $all_selections = array();
  $first_selection = null;
  $first_selections = array();
  /*
   * create the render arrays for each case
   */
  switch ($table_selection_type) {
  	// for all three of these cases, use the HTML tableselect header and options
    case 'copy_single': // single row selection and key field copy via radio buttons
    case 'copy_multiple': // multiple row selection and key field copy via check boxes
    case 'delete_multiple': // multiple row selection and record deletion via check boxes
      $header = array();
      $options = array();
      for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
        $row = array();
        $options_key = null;
        $column_count = 0;
        for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
          $db_value = fetchit_get_data($db_type, $db_data_object, $row_ndx, $field_ndx);
          if ($field_ndx == ($uid_field_ndx - 1)) {
            // don't do anything with this value
          }
          else {
            if ($field_ndx == ($key_field_ndx - 1)) {
              // use this as the key for the options array
              $options_key = $db_value;
              // it's the index field
              $all_selections[$db_value] = TRUE;
            }
            else {
              if ($row_ndx == 0) {
                // it's the first row
                $first_selection = $db_value;
                $first_selections[$db_value] = TRUE;
                if ($column_count < $table_headers_count) {
                  $header['column' . $column_count] = $table_headers[$column_count];
                }
                else {
                  $header['column' . $column_count] = $field_names[$field_ndx];
                }
              }
              $row['column' . $column_count] = $db_value;
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
      }
      else {
        $multiple_flag = TRUE;
        $selectall_flag = TRUE;
      }
      // set the default based on the type and the flag
      switch ($table_selection_type) {
        case 'copy_single':
          switch ($table_selection_default_flag) {
            case '-':
              $default_value = $first_selection;
              break;
            case '*':
              $default_value = $previous_selection;
              break;
            case '+':
              $default_value = $first_selection;
              break;
            default:
              $default_value = $first_selection;
          }
          break;
        case 'copy_multiple':
          switch ($table_selection_default_flag) {
            case '-':
              $default_value = $none_selections;
              break;
            case '*':
              $default_value = $previous_selections;
              break;
            case '+':
              $default_value = $all_selections;
              break;
            default:
              $default_value = $none_selections;
          }
          break;
        case 'delete_multiple':
          switch ($table_selection_default_flag) {
            case '-':
              $default_value = $none_selections;
              break;
            case '*':
              $default_value = $none_selections;
              break;
            case '+':
              $default_value = $all_selections;
              break;
            default:
              $default_value = $none_selections;
          }
          break;
        default:
          $default_value = null;
      }
      // check the default value created
      // debug($default_value, '$default_value');
      /* 
       * create the HTML tableselect render array with a single select option
       */
      $form['table'] = array(
		'#type' => 'tableselect',
      	'#header' => $header,
      	'#options' => $options,
      	'#default_value' => $default_value,
      	'#multiple' => $multiple_flag,
      	'#js_select' => $selectall_flag,
      	'#empty' => $table_empty_message,
      	'#weight' => $table_weight,
	  );
	  // no submit button if table is empty
	  // if ($row_count==0) $submit_button_flag = FALSE;
      break;
    // for all other cases, create a simple html table (NOT a tableselect)
    case 'none':
    default:
      $header = array();
      $rows = array();
      for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
        $row = array();
        $column_count = 0;
        for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
          if ($field_ndx != ($uid_field_ndx - 1)) {
            if ($field_ndx != ($key_field_ndx - 1)) {
              if ($row_ndx == 0) {
                if ($column_count < $table_headers_count) {
                  $header[] = $table_headers[$column_count];
                }
                else {
                  $header[] = $field_names[$field_ndx];
                }
              }
              $db_value = fetchit_get_data($db_type, $db_data_object, $row_ndx, $field_ndx);
              $row[] = $db_value;
              ++$column_count;
            }
          }
        }
        $rows[] = $row;
      }
      /*
       * create the simple HTML table render array
       */
      $form['table'] = array(
      		'#theme' => 'table',
      		'#header' => $header,
      		'#rows' => $rows,
      		'#empty' => $table_empty_message,
      		'#weight' => $table_weight,
      );
  }

  // append record elements
  if ($append_record_flag) {
    $form['fetchit7_table_text'] = array('#type' => 'textfield', '#default_value' => variable_get('fetchit7_table_text', 'fetchit7_table_text_default_value'), '#title' => t('Enter some text'), '#size' => 30, '#maxlength' => 60, '#required' => TRUE, );
  }

  // parameter select element
  if ($parameter_select_flag) {
    $form['fetchit7_table_text'] = array('#type' => 'textfield', '#default_value' => variable_get('fetchit7_table_text', 'fetchit7_table_text_default_value'), '#title' => t('Enter some text'), '#size' => 30, '#maxlength' => 60, '#required' => TRUE, );
  }

  // text input element
  if ($text_input_flag) {
    $form['fetchit7_table_text'] = array('#type' => 'textfield', '#default_value' => variable_get('fetchit7_table_text', 'fetchit7_table_text_default_value'), '#title' => t('Enter some text'), '#size' => 30, '#maxlength' => 60, '#required' => TRUE, );
  }

  if ($addbody_flag) {
    $body = fetchit7_content_field_fetcher($nid, 'body', 'value');
    $form['body'] = array('#type' => 'markup', '#markup' => $body, );
  }

  $form['submit_button_flag'] = array('#type' => 'value', '#value' => $submit_button_flag, );
  if ($submit_button_flag) {
    $form['submit'] = array('#type' => 'submit', '#value' => $submit_label, '#weight' => $submit_weight, );
  }

  //$form['#validate'][] = 'fetchit7_table_form_validate';
  //$form['#submit'][] = 'fetchit7_table_form_submit';

  return $form;

}

/**
 * Process form validation
 */
function fetchit7_table_form_validate($form, &$form_state) {
  // get the form nid
  $nid = $form['nid']['#value'];
  //debug($nid,'fetchit7_table_form_validate the nid is ');
  // get the table type
  $record_select = $form['record_select']['#value'];
	//debug($record_select,'validate $record_select');
  // check out the table for errors
  switch ($record_select) {
    case 0:
      // nothing really to check on a plain html table
      break;
    case 1:
      //radio buttons
      $selected = $form_state['values']['table'];
			//debug($selected,'validate $selected');
      if (is_null($selected)) {
        form_set_error('submit', t("Invalid selection: $selected  A selection is required before pressing the submit button."));
        return;
      }
      $key_field = $form['key_field']['#value'];
      if (!strlen($key_field)) {
        form_set_error('submit', t('fetchit7 setup error: missing key field name.  A key field name is required.'));
        return;
      }
      $record_select_table = $form['record_select_table']['#value'];
      if (!strlen($record_select_table)) {
        form_set_error('submit', t('fetchit7 setup error: missing selection table name.  A selection table name is required.'));
        return;
      }
      break;
    case 2:
      //checkboxes for record selection
      $key_field = $form['key_field']['#value'];
      if (!strlen($key_field)) {
        form_set_error('submit', t('fetchit7 setup error: missing key field name.  A key field name is required.'));
        return;
      }
      $record_select_table = $form['record_select_table']['#value'];
      if (!strlen($record_select_table)) {
        form_set_error('submit', t('fetchit setup error: missing selection table name.  please contact the site administrator.'));
        return;
      }
      break;
    case 3:
      //checkboxes for record deletion
      $key_field = $form['key_field']['#value'];
      if (!strlen($key_field)) {
        form_set_error('submit', t('fetchit7 setup error: missing key field name.  A key field name is required.'));
        return;
      }
      break;
    default:
  }
}

/**
 * Process form submission
 */
function fetchit7_table_form_submit($form, &$form_state) {
  $nid = $form['nid']['#value'];
  //debug($nid,'fetchit7_table_form_submit the nid is ');
  $record_select = $form['record_select']['#value'];
  $db_type = $form['db_type']['#value'];
  $db_handle = $form['db_handle']['#value'];
	//debug($record_select,'submit $record_select');
  switch ($record_select) {
    case 'none':
      // static html - no selection - do nothing
      break;
    case 'copy_single':
      // there is always one value when it is working correctly
      $key_field_ndx = $form['key_field_ndx']['#value'];
			//debug($key_field_ndx,'submit $key_field_ndx');
      if ($key_field_ndx) {
        $db_type = $form['db_type']['#value'];
        $db_handle = $form['db_handle']['#value'];
        $selected_radio_option = $form_state['values']['table'];
				//debug($selected_radio_option,'submit $selected_radio_option');
				//have to handle a special case here - single select table but table is empty
	      $row_count = $form['row_count']['#value'];
				//debug($row_count,'submit $row_count');
				if ($row_count==0) {
	      //    drupal_set_message(t("Selection not saved because selection table was empty."), 'warning');
				}	else {
	        if (is_null($selected_radio_option)) {
	          drupal_set_message(t("Selection not saved because no records were selected."), 'warning');
	        }
	        else {
	          // now the D7 tableselect actually returns the key field value
	          $key_field_val = $selected_radio_option;
	          if ($result = fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $key_field_val)) {
	            drupal_set_message(t("Selection saved."), 'status');
	          }
	          else {
	            drupal_set_message(t("The selected record was NOT saved. Please contact the site administrator."), 'warning');
	          }
	        }
				}			
      }
      else {
        drupal_set_message(t("The selected record was NOT saved because a table key field was not detected.  Please contact the site administrator."), 'warning');
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
          $result = fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $key_field_vals);
          if ($result) {
            drupal_set_message(t("Selections ($result records) saved."), 'status');
          }
          else {
            drupal_set_message(t("The selected records were NOT saved. Please contact the site administrator."), 'warning');
          }
        }
        else {
          $result = fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $key_field_vals);
          drupal_set_message(t("Selections ($result records) saved."), 'status');
        }
      }
      else {
        drupal_set_message(t("The selected records were NOT saved because a table key field was not detected.  Please contact the site administrator."), 'warning');
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
          $result = fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $key_field_vals);
          if ($result) {
            drupal_set_message(t("Selections ($result records) deleted."), 'status');
          }
          else {
            drupal_set_message(t("The selected records were NOT deleted. Please contact the site administrator."), 'warning');
          }
        }
        else {
          $result = fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $key_field_vals);
          drupal_set_message(t("Selections ($result records) deleted."), 'status');
        }
      }
      else {
        drupal_set_message(t("The selected records were NOT deleted because a table key field was not detected.  Please contact the site administrator."), 'warning');
      }
      break;
    default:
  }

  //run the post query
  $postqry = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_post_function', 'value'));
  if (strlen($postqry)) {
    if (strtolower(substr($postqry, 0, 4)) == 'php.') {
      $thefunction = substr($postqry, 4);
      global $user;
      $user_uid = (int)$user -> uid;
      $result = $thefunction($user_uid);
    }
    else {
      $result = fetchit_run_query($db_type, $db_handle, $postqry);
    }
  }

  // redirect to the next page
  $redirect = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_redirect', 'value'));
  if (strlen($redirect)) {
    $form_state['redirect'] = $redirect;
  }

  // reset the text
  drupal_set_message(t('Successfully submitted changes.'), 'status');
}

function theme_fetchit7_table_form($variables) {
  $form = $variables['form'];
  $form_id = $form['form_id']['#value'];
  $output = '<div class="' . $form_id . '_table">';
  $output .= drupal_render($form['table']);
  $output .= '</div>';
	$submit_button_flag = $form['submit_button_flag']['#value'];
	if ($submit_button_flag) {
    $output .= '<div class="' . $form_id . '_submit">';
		$output .= drupal_render($form['submit']);
    $output .= '</div>';
	}
	$addbody_flag = $form['addbody_flag']['#value'];
	if ($addbody_flag) {
    $output .= '<div class="' . $form_id . '_body">';
		$output .= drupal_render($form['body']);
    $output .= '</div>';
	}
  $output .= '<div class="' . $form_id . '_other">';
  $output .= drupal_render_children($form);
  $output .= '</div>';
  return $output;
}

//===================================
// fetchit7 parameter input form content
//===================================

/**
 * Form builder.  Setup the elements (controls) to be displayed on the fetchit7 parameter input form .
 *
 * @ingroup forms
 * @see system_settings_form().
 *
 * this function creates the parameter data input form
 * first it gets the data stored in the drupal node and content type tables (created in the .install file)
 * that define what the form will look like
 * this is done via a function, fetchit7_content_field_fetcher(), that calls the drupal API functions for extracting node and field data
 * next it stores that data in a special array, $user_field_data, that was designed when fetchit2 was created
 * using this array maximizes the amount of fetchit2 code that can be reused
 * that array is used with a special form builder function, fetchit7_param_element_form,
 * that calls a bunch of element (form controls) creation functions
 * this collection of form elements is pre-rendered and then arranged on the form
 * that the user uses to enter the various parameter data or make parameter value selections
 *
 */
function fetchit7_param_form($form, &$form_state) {
  $addbody_flag = TRUE; // until we have a flag for this it on the fetchit7 form
  // some things we can put in the $form
  //$form['#attached']['css'] = array(drupal_get_path('module', 'block') . '/block.css');
  $form['elements'] = array();
  $form['#tree'] = TRUE;

  // get nid embedded into original form_id; format fetchit7_param_form_X### - nid starts at character 21
  $form_id = $form_state['build_info']['form_id'];
  //debug($form_id, microtime(true).' fetchit7_param_form: $form_id:');
  // to get here, this is definitely set
  //debug($form_state['build_info']);
  $nid = (int) substr($form_id, 21);
  $form['nid'] = array('#type' => 'value', '#value' => $nid);

  // get database type and store it (hidden) in the form so it is available to the form validate and submit functions
  // this is not optional, and can not be filled in with a default, so if it is missing, throw an error and exit
  // this should be a single string
  $db_type = trim(fetchit7_content_field_fetcher($nid, 'db_type', 'value'));
  if (strlen($db_type)) {
    $form['db_type'] = array('#type' => 'value', '#value' => $db_type);
  }
  else {
    drupal_set_message("An error occurred in fetchit7_param_form (nid = $nid) when accessing the database type.", 'error');
    return $form;
  }

  // get database handle
  if ($db_handle = fetchit_get_db_handle($nid)) {
    $form['db_handle'] = array('#type' => 'value', '#value' => $db_handle);
    //debug($db_handle,'$db_handle');
  }
  else {
    drupal_set_message("An error occurred in fetchit7_param_form (nid = $nid) when creating the database connection handle.", 'error');
    return $form;
  }

  // run the pre query before doing anything else
  // TODO: figure out why pre-fn is getting hit twice prior to the form drawing!!!
  // get query string. this is optional, so if it is missing, move on
  // this should be a single string
  $preqry = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_pre_function', 'value'));
  if (strlen($preqry)) {
    if (strtolower(substr($preqry, 0, 4)) == 'php.') {
      $thefunction = substr($preqry, 4);
      global $user;
      $user_uid = (int)$user -> uid;
      $result = $thefunction($user_uid);
    }
    else {
      $result = fetchit_run_query($db_type, $db_handle, $preqry);
    }
    //TODO: check pre function result?
  }

  // addbody
  // addbody_flag should be a single boolean value
  // use it if something valid is entered.   if not set it to a default of true
  //$addbody_flag = fetchit7_content_field_fetcher($nid, 'addbody_flag', 'value');
  $form['addbody_flag'] = array('#type' => 'value', '#value' => $addbody_flag);

  // submit button attributes
  // submit button label should be a single string value
  // use it if something is entered.   if not offer a default of Submit
  $submit_label = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_label', 'value'));
  if (strlen($submit_label)) {
    $submit_flag = TRUE;
  }
  else {
    $submit_flag = TRUE;
    $submit_label = 'Submit';
    // in case we HAVE to have a button
  }
  $form['submit_label'] = array('#type' => 'value', '#value' => $submit_label);

  // submit button weight should be a single integer value
  // use it if something is entered.   if not offer a default of 10 - puts it at or close to the bottom of the form
  $submit_weight = fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_weight', 'value');
  $form['submit_weight'] = array('#type' => 'value', '#value' => $submit_weight);

  // redirectl should be a single string value
  // optional, use it if something is entered.   if not set it to an empty string
  $submit_redirect = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_redirect', 'value'));
  $form['submit_redirect'] = array('#type' => 'value', '#value' => $submit_redirect);

  // table attributes
  // showtitles should be a single boolean value
  // use it if something valid is entered.   if not set it to a default of true
  $showtitles = fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_shw_ttl', 'value');
  $form['showtitles'] = array('#type' => 'value', '#value' => $showtitles);

  // parameter table weight should be a single integer value
  $table_weight = fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_weight', 'value');
  $form['table_weight'] = array('#type' => 'value', '#value' => $table_weight);

  // parameter table_rows should be a single integer value
  $table_rows = fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_tbl_rows', 'value');
  //debug($table_rows,'param_form $table_rows');
  $form['table_rows'] = array('#type' => 'value', '#value' => $table_rows);
  //debug($form['table_rows'],'param_form $form[table_rows]');

  // parameter table_headers should be an array of text values
  // warning: d7 does not return empty strings at all
  // if someone put spaces as a placeholder, trim them to an empty string
  $table_headers = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_headers', 'values'));
  //should return an array
  $form['table_headers'] = array('#type' => 'value', '#value' => $table_headers);

  // fetchit7_content_field_fetcher() returns the type of the underlying field,
  // So these should all be arrays of string values entered by the user
  //    used these because they are the only multiple valued multiple control form type available in D7 core
  // They are arrays therefore pass the values format tag to fetchit7_content_field_fetcher()
  // They are strings, so there will need to do some type conversion later
  // Depending on what the user entered, the arrays are potentially different sizes
  // warning: d7 does not return empty strings at all
  // These are preprocessed (cleaned up and converted to the appropriate variable type)
  // and stored in the $user_field_data array below
  // check that they are defined and are arrays.  some can not be empty...
  $param_names_raw = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_name', 'values'));
  //should return an array
  $param_names = array();
  $foundone = FALSE;
  foreach ($param_names_raw as $param_name) {
    if (strlen($param_name)) {
      $param_names[] = $param_name;
      $foundone = TRUE;
    }
  }
  if (!$foundone) {
    drupal_set_message("An error occurred creating the parameter input form (nid = $nid).  No parameter names were specified.", 'error');
    return $form;
  }
  // optional
  //should return an array
  $param_titles = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_title', 'values'));
  //should return an array
  $param_descriptions = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_desc', 'values'));
  // i equivocate on this, but for now is optional - there is a default control type that is picked based on the output field type
  //should return an array
  $param_types = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_type', 'values'));
  // weights are integers, but leave these input values as strings until later
  //should return an array
  $param_weights = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_weight', 'values'));
  //debug($param_weights,'$param_weights');
  // boolean, but leave these input values as strings until later, also can deal with default values then
  //should return an array
  $param_enabled = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_enabled', 'values'));
  // boolean, but leave these input values as strings until later, also can deal with default values then
  //should return an array
  $param_required = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_required', 'values'));

  // optional
  //should return an array
  $param_str = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_parstr', 'values'));

  //should return an array
  $param_list_tbl = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_tbl', 'values'));
  //should return an array
  $param_list_key = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_key', 'values'));
  //should return an array
  $param_list_dsp = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_dsp', 'values'));
  //should return an array
  $param_list_uid = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_uid', 'values'));
  //should return an array
  // note that the default strings in this array might have a "special" initial char, which will be handled later
  $param_list_def = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_def', 'values'));
  
  //should return an array
  // note that the database table names in this array might have a "special" initial char, which will be handled later
  $param_data_tbl = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_data_tbl', 'values'));
  //should return an array
  $param_data_key = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_data_key', 'values'));
  //should return an array
  $param_data_uid = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_data_uid', 'values'));

  //use the above arrays to contruct the $user_field_data array allowing the use of the fetchit2 code AMAP
  // note that param names DEFINES the count of the fields
  // we will use defaults is possible where items are missing
  // exit gracefully with error message otherwise
  $control_list = fetchit7_get_control_list();
  $control_list_string = fetchit7_get_control_list_string();

  $user_field_data = array();
  $field_index = 0;
  // even though the fetchit2 does this too...do some data preprocessing
  foreach ($param_names as $param_name) {// they will NOT be blank at this point
    $user_field_data[$field_index][F_DR_FIELD_NAME_STR] = $param_name;

    //title default of '' is OK
    if ($field_index < count($param_titles)) {
      $user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = $param_titles[$field_index];
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = '';
    }

    //description default of '' is OK
    if ($field_index < count($param_descriptions)) {
      $user_field_data[$field_index][F_DR_FIELD_DESC_STR] = $param_descriptions[$field_index];
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DESC_STR] = '';
    }

    // types require some more checking since they are typed into a text box
    if ($field_index < count($param_types)) {
      // see if the control type matches something we know about
      if ($key = array_search(strtolower($param_types[$field_index]), $control_list)) {
        $user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = $key - 1;
        // the fetchit2 code wants an index here, and we had incremented these by 1 above to make array_search() easier to use
      }
      else {
        drupal_set_message("Parameter $field_index control type (" . $param_types[$field_index] . ") unrecognized.  Valid control types are [$control_list_string]. Please contact the site administrator.", 'error');
        return;
      }
    }
    else {
      // can't have a default here, too many complications
      drupal_set_message("Parameter $field_index control type missing.  There must be as many control types specified as there are parameter names. Please contact the site administrator.", 'error');
      return;
    }

    //weight default of param_index is OK (defaults to the order they were created)
    if ($field_index < count($param_weights)) {
      $user_field_data[$field_index][FETCHIT7_FIELD_WEIGHT_STR] = (int)$param_weights[$field_index];
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_WEIGHT_STR] = $field_index;
    }

    //$param_enabled
    //fetchit2 uses disabled - so default of disabled = true is OK
    if ($field_index < count($param_enabled)) {
      $enabled = filter_var($param_enabled[$field_index], FILTER_VALIDATE_BOOLEAN);
      $user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = !$enabled;
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = TRUE;
    }

    //$param_required
    //default of required = true is OK
    if ($field_index < count($param_required)) {
      $user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = filter_var($param_required[$field_index], FILTER_VALIDATE_BOOLEAN);
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = TRUE;
    }

    //$param_str parameter string
    //title default of '' is OK
    if ($field_index < count($param_str)) {
      $user_field_data[$field_index][F_DR_FIELD_PARAMETER_STR] = $param_str[$field_index];
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_PARAMETER_STR] = '';
    }

    //$param_list_tbl F_DR_FIELD_LIST_TBL_STR
    //table name default of '' is OK
    if ($field_index < count($param_list_tbl)) {
      $user_field_data[$field_index][F_DR_FIELD_LIST_TBL_STR] = $param_list_tbl[$field_index];
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_LIST_TBL_STR] = '';
    }

    //$param_list_key
    if ($field_index < count($param_list_key)) {
      if (strlen($param_list_key[$field_index])) {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = $param_list_key[$field_index];
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = F_DR_LIST_TBL_NDX_FLD;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = F_DR_LIST_TBL_NDX_FLD;
    }

    //$param_list_dsp
    if ($field_index < count($param_list_dsp)) {
      if (strlen($param_list_dsp[$field_index])) {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = $param_list_dsp[$field_index];
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = F_DR_LIST_TBL_DSP_FLD;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = F_DR_LIST_TBL_DSP_FLD;
    }

    //$param_list_uid F_DR_FIELD_LIST_UID_STR
    // uid field name - blank of not used - default of blank is OK (usually the items in a parameter selection list are not user specific)
    if ($field_index < count($param_list_uid)) {
      $user_field_data[$field_index][F_DR_FIELD_LIST_UID_STR] = $param_list_uid[$field_index];
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_LIST_UID_STR] = '';
    }

    //$param_list_def F_DR_FIELD_DEFAULT_STR
    // adding something new here - if the first char is a special character:
    //   - means put all and none buttons above, 
    //   * and + means put all and none buttons below
    // also strip the special allnone button character from the default value if it is there (- * +)
    if ($field_index < count($param_list_def)) {
      if (strlen($param_list_def[$field_index])) {
        $user_field_data[$field_index][FETCHIT7_ALLNONE_SPECIAL_CHAR_STR] = fetchit7_get_special_char_allnone($param_list_def[$field_index]);
        $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = fetchit7_strip_special_char_allnone($param_list_def[$field_index]);
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        $user_field_data[$field_index][FETCHIT7_ALLNONE_SPECIAL_CHAR_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
      $user_field_data[$field_index][FETCHIT7_ALLNONE_SPECIAL_CHAR_STR] = '';
    }

    //$param_data_tbl F_DR_FIELD_DATA_TBL_STR
    // this is a required value
    // adding something new here - if the first char is a special character:
    //   - means default selection is none
    //   * means default selection is previous selection
    //   + means default selection is all
    // also strip the special character from the table name if it is there (- * +)
    //   only doing this because it could be confusing to the fetchit user to have special characters
    //   on selectable tables but not parameters - so support it on both, even though it is not
    //   necessary on parameters
    if ($field_index < count($param_data_tbl)) {
      if (strlen($param_data_tbl[$field_index])) {
        $user_field_data[$field_index][FETCHIT7_DEFAULT_SPECIAL_CHAR_STR] = fetchit7_get_special_char_default($param_data_tbl[$field_index]);
        $user_field_data[$field_index][F_DR_FIELD_DATA_TBL_STR] = fetchit7_strip_special_char_default($param_data_tbl[$field_index]);
        // if we have a special character on the table name
        if (strlen($user_field_data[$field_index][FETCHIT7_DEFAULT_SPECIAL_CHAR_STR])) {
        	// then set the default via the param default field if it isn't set already
        	if ($user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR]=='') {
        		switch ($user_field_data[$field_index][FETCHIT7_DEFAULT_SPECIAL_CHAR_STR]) {
        			case '-':
        				// unfortunately have to know the param type to know the appropriate default
        				switch ($user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR]) {
        					case F_DR_CONTROL_DROPDOWN_SINGLE:
        					case F_DR_CONTROL_RADIOBTN:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_FIRST;
        						break;
        					case F_DR_CONTROL_CHECKBOXES:
        					case F_DR_CONTROL_DROPDOWN_MULTI:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_NONE;
        						break;
        					case F_DR_CONTROL_CHECKBOX:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_FALSE;
        						break;
        					default:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        				}
        				break;
        			case '*':
        				// unfortunately have to know the param type to know the appropriate default
        				switch ($user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR]) {
        					case F_DR_CONTROL_DROPDOWN_SINGLE:
        					case F_DR_CONTROL_RADIOBTN:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_PREVIOUS;
        						break;
        					case F_DR_CONTROL_CHECKBOXES:
        					case F_DR_CONTROL_DROPDOWN_MULTI:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_PREVIOUS;
        						break;
        					case F_DR_CONTROL_CHECKBOX:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_PREVIOUS;
        						break;
        					default:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        				}
        				break;
        			case '+':
        		        // unfortunately have to know the param type to know the appropriate default
        				switch ($user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR]) {
        					case F_DR_CONTROL_DROPDOWN_SINGLE:
        					case F_DR_CONTROL_RADIOBTN:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_FIRST;
        						break;
        					case F_DR_CONTROL_CHECKBOXES:
        					case F_DR_CONTROL_DROPDOWN_MULTI:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_ALL;
        						break;
        					case F_DR_CONTROL_CHECKBOX:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = FETCHIT7_FIELD_DEFAULT_TRUE;
        						break;
        					default:
        						$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        				}
        				break;
        			default:
        				$user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        		}
        	}
        }
      }
      else {
        // can't have a default here, too many complications
        drupal_set_message("Parameter $field_index data selection table missing.  There must be as many data selection tables specified as there are parameter names. Please contact the site administrator.", 'error');
        return;
      }
    }
    else {
      // can't have a default here, too many complications
      drupal_set_message("Parameter $field_index data selection table missing.  There must be as many data selection tables specified as there are parameter names. Please contact the site administrator.", 'error');
      return;
    }

    //$param_data_key - we must have a key name - use stored as the default
    if ($field_index < count($param_data_key)) {
      if (strlen($param_data_key[$field_index])) {
        $user_field_data[$field_index][F_DR_FIELD_DATA_KEY_STR] = $param_data_key[$field_index];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DATA_KEY_STR] = F_DR_DATA_TBL_NDX_FLD;
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DATA_KEY_STR] = F_DR_DATA_TBL_NDX_FLD;
    }

    //$param_data_uid F_DR_FIELD_DATA_UID_STR
    // used in submit function code, it is the uid field name
    if ($field_index < count($param_data_uid)) {
      if (strlen($param_data_key[$field_index])) {
        $user_field_data[$field_index][F_DR_FIELD_DATA_UID_STR] = $param_data_uid[$field_index];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DATA_UID_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DATA_UID_STR] = '';
    }
    ++$field_index;
  }
  $field_count = $field_index;

  // loop through the user entered fields and make a table of parameter input control elements
  // render each control element then put the rendered elements into an unrendered table
  // critical to the operation of this form are the unique form ids of the rendered control elements
  $param_count = 0;
  //for ($i=0; $i<F_DR_PARAM_FIELD_LIST_COUNT; $i++) {
  for ($i = 0; $i < $field_count; $i++) {
    // param types are the field types of the "stored" (F_DR_DATA_TBL_NDX_FLD) field in the data tables where the parameter values are saved
    // and probably (hopefully? required?) the same type as the stored values in the list tables if they are used
    // regardless, the control will be set-up to handle the data table field type
    // and whatever value is gotten from the control will be formatted to fit into the data table field
    // also, we ASSUME there is also a Drupal user_id field "drupal_userid" (F_DR_DATA_TBL_UID_FLD) in EVERY parameter data table
    $data_tbl = $user_field_data[$i][F_DR_FIELD_DATA_TBL_STR];
    //$data_ndx = F_DR_DATA_TBL_NDX_FLD;
    $data_ndx = $user_field_data[$i][F_DR_FIELD_DATA_KEY_STR];
    //$data_uid = F_DR_DATA_TBL_UID_FLD;
    $data_uid = $user_field_data[$i][F_DR_FIELD_DATA_UID_STR];
    if (strlen(trim($data_tbl))) {// if a data table is specified then we can continue building the parameter input control
      //just need field names and types, does not really matter which record we get
      $data_tbl_db_object = fetchit_get_data_object_1($db_type, $db_handle, $data_tbl, $data_uid);
      $data_tbl_field_names = fetchit_get_field_names($db_type, $data_tbl_db_object);
      $data_tbl_field_types = fetchit_get_field_types($db_type, $data_tbl_db_object);
      $j = 0;
      //find the index field and get its type
      $data_tbl_ndx_type = '';
      foreach ($data_tbl_field_names as $data_tbl_field_name) {
        if ($data_tbl_field_name == $data_ndx) {
          $data_tbl_ndx_type = $data_tbl_field_types[$j];
          break;
        }
        $j++;
      }
      //debug($data_tbl_ndx_type,'fetchit7_param_form $data_tbl_ndx_type');
      // if we have a type, then an index field was found,
      //   and then we can continue building the parameter input control
      if (strlen(trim($data_tbl_ndx_type))) {
        $grp_ndx = 0;
        //create a unique element name string
        $element_name = fetchit_make_element_name($nid, $grp_ndx, $param_count);
        // set a default value to pass into the element generator
        // this value is used unless the user specified a default value in the configuration
        // get the last entered value from the selection table
        // depending on the control type, this is either a single value or an array of values...
        switch ($user_field_data[$i][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_CHECKBOXES:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          	// first get the previously saved state...
            $db_value = fetchit_get_existing_selection_multiple($db_handle, $db_type, $data_tbl, $data_ndx, $data_uid);
			//debug($db_value,microtime(true).' creating form element '.$element_name.'. Default value set to');
            // next, check to see if there is an "interim state" to reset the control too
	        //   (this happens when the user presses a select All or select None button)
	        //   see if there is a stored variable with the name element_name_value
			if (variable_get($element_name.'_set',FALSE)) {
				// need to reset this control to an interim saved state
				// set the default string to FETCHIT7_FIELD_DEFAULT_PREVIOUS
				//    and then set $db_value to the interim state, overwriting the previously selected state
				//    then set the $user_field_data[] flag so that the fetchit 2 routines will use it,
				//  essentially this is fooling the fetchit2 routines into thinking the interim state
				//    is the previously selected state
        		$user_field_data[$i][F_DR_FIELD_DEFAULT_STR]=FETCHIT7_FIELD_DEFAULT_PREVIOUS;
				$db_value = variable_get($element_name.'_value','');
			    //debug($db_value,microtime(true).' creating form element '.$element_name.'. RESETTING default value to');
				//and then finally erase the interim state flag...
				variable_set($element_name.'_set',FALSE);
			}
            // finally, here is where we change the default if the all or none buttons were pressed for this element
            //   for this particular element
            // at most only one of these should be true, but handle both separately so they get reset correctly,
            //   just in case...
            if (variable_get($element_name.'_allbtn',FALSE)) {
            	$user_field_data[$i][F_DR_FIELD_DEFAULT_STR]=FETCHIT7_FIELD_DEFAULT_ALL;
            	variable_set($element_name.'_allbtn',FALSE);
            }
            if (variable_get($element_name.'_nonebtn',FALSE)) {
            	$user_field_data[$i][F_DR_FIELD_DEFAULT_STR]=FETCHIT7_FIELD_DEFAULT_NONE;
            	variable_set($element_name.'_nonebtn',FALSE);
            }
            // only need to make these if it is this type of control
            // be sure to check isset in theme function, though
            switch ($user_field_data[$i][FETCHIT7_ALLNONE_SPECIAL_CHAR_STR]) {
            	case '-':
            	case '*':
            	case '+':
			        $form['elements'][$element_name.'_allbtn'] = array(
			        		'#type' => 'submit',
			        		'#name' => $element_name.'_allbtn',
			        		//'#validate' => array(
			        		//		'fetchit7_param_all_validate'
			        		//),
			        		'#limit_validation_errors' => array(),
			        		'#submit' => array(
			        				'fetchit7_param_all_submit'
			        		),
			        		'#buttontype' => 'button',
			        		'#value' => t('All'),
			        );
			        $form['elements'][$element_name.'_nonebtn'] = array(
			        		'#type' => 'submit',
			        		'#name' => $element_name.'_nonebtn',
			        		'#limit_validation_errors' => array(),
			        		//'#validate' => array(
			        		//		'fetchit7_param_none_validate'
			        		//),
			        		'#submit' => array(
			        				'fetchit7_param_none_submit'
			        		),
			        		'#buttontype' => 'button',
			        		'#value' => t('None'),
			        );
            		break;
            	default:
            }
            break;
          case F_DR_CONTROL_DROPDOWN_SINGLE:
          case F_DR_CONTROL_RADIOBTN:
          case F_DR_CONTROL_TEXTBOX:
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_DATESEL:
          case F_DR_CONTROL_DATETIMESEL:
          case F_DR_CONTROL_POPUPDATE:
            $db_value = fetchit_get_existing_selection_single($db_handle, $db_type, $data_tbl, $data_ndx, $data_uid);

            //debug($db_value,microtime(true).' creating form element '.$element_name.'. Default value set to');
            // next, check to see if there is an "interim state" to reset the control too
            //   (this happens when the user presses a select All or select None button)
            //   see if there is a stored variable with the name element_name_value
            if (variable_get($element_name.'_set',FALSE)) {
            	// need to reset this control to an interim saved state
            	// set the default string to FETCHIT7_FIELD_DEFAULT_PREVIOUS
            	//    and then set $db_value to the interim state, overwriting the previously selected state
            	//    then set the $user_field_data[] flag so that the fetchit 2 routines will use it,
            	//  essentially this is fooling the fetchit2 routines into thinking the interim state
            	//    is the previously selected state
            	$user_field_data[$i][F_DR_FIELD_DEFAULT_STR]=FETCHIT7_FIELD_DEFAULT_PREVIOUS;
            	$db_value = variable_get($element_name.'_value','');
            	//debug($db_value,microtime(true).' creating form element '.$element_name.'. RESETTING default value to');
            	//and then finally erase the interim state flag...
            	variable_set($element_name.'_set',FALSE);
            }
            break;
          default:
        }
        // add everything else to the form we might need in the theme, validate, or submit functions
        $form['param_count'] = array('#type' => 'value', '#value' => ($param_count + 1));
        // update this each time through
        $form['param_indices'][$param_count] = array('#type' => 'value', '#value' => $i);
        $form['field_types'][$param_count] = array('#type' => 'value', '#value' => $data_tbl_ndx_type);
        $form['param_names'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_NAME_STR]);
        $form['param_types'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_INPUT_TYPE_STR]);
        $form['param_titles'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_TITLE_STR]);
        $form['param_descriptions'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DESC_STR]);
        $form['param_weights'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][FETCHIT7_FIELD_WEIGHT_STR]);
        $form['param_data_table'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DATA_TBL_STR]);
        $form['param_data_ndx'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DATA_KEY_STR]);
        $form['param_data_uid'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DATA_UID_STR]);
        // note that for the following - would like to eventually add the option for the all none buttons to be added automatically based on the size of the list
        $form['param_allnone'][$param_count] = array('#type' => 'value', '#value' => $user_field_data[$i][FETCHIT7_ALLNONE_SPECIAL_CHAR_STR]);
        
        // the following creates the "renderable array" of the next parameter input control (drupal form element)
        //   to be displayed on this form ($nid)
        // stack these up in the array. "How" they are to be displayed will be handled by the theme function
        $form['elements'][$element_name] = 
        	fetchit7_param_element_form(
        			$param_name,
        			$data_tbl_ndx_type,
        			$db_handle,
        			$i + 1,
        			$user_field_data,
        			$nid,
        			$showtitles,
        			$db_value);
        //debug($myform[$element_name],' pre-render $myform['.$element_name.']');
        $param_count++;
      }
    }
  }
  /*
   // choose whether to output form elements in a table ($table_rows>0)
   // 	or put them in a stack accd to weight ($table_rows=0)
   if ($table_rows) { //pre-render the elements and put them in a table
   // this form will be a table of pre-rendered controls - hope it works!
   $table_column_count =  intval( ($param_count-1) / $table_rows) + 1;
   $header = array();
   for ($col_ndx = 0; $col_ndx < $table_column_count; $col_ndx++) {
   // TODO: need to add column header labels to parameter input form inputs
   $header[] = "Column $col_ndx";
   }
   $rows = array();
   for ($row_ndx = 0; $row_ndx < $table_rows; $row_ndx++) {
   $row = array();
   for ($col_ndx = 0; $col_ndx < $table_column_count; $col_ndx++) {
   // numbering goes down the columns first
   $p_ndx = $row_ndx + $col_ndx * $table_rows;
   if ($p_ndx < $param_count) {
   $element_name = fetchit_make_element_name($nid,0,$p_ndx);
   $row[] = drupal_render($myform[$element_name]);
   //debug(drupal_render($myform[$element_name]),' post-render $myform['.$element_name.']');
   } else {
   $sillyform[$p_ndx] = array('#type' => 'item','#title' => 'Empty Cell','#markup' => "I am an empty cell.",);
   $row[] = drupal_render($sillyform[$p_ndx]);
   //$row[] = $p_ndx;
   }
   }
   $rows[] = $row;
   }
   $form['table'] = array(
   '#theme' => 'table',
   '#header' => $header,
   '#rows' => $rows,
   );
   } else { // just output the form elements in their weight order
   $form['elements'] = $myform;
   }
   */

  $submit_button_flag = TRUE;
  $form['submit_button_flag'] = array('#type' => 'value', '#value' => $submit_button_flag);
  if ($submit_button_flag) {
    //$form['actions'] = array(
    //  '#tree' => FALSE,
    //  '#type' => 'actions',
    //);
    //debug($submit_weight,'$submit_weight');
    $form['actions']['submit'] = array(
    		'#type' => 'submit',
    		'#value' => $submit_label,
    		'#weight' => $submit_weight, );
  }
  $form['#after_build'][] = 'fetchit7_param_form_force_triggering_element';
  return $form;
}

//this function is called before drupal processes the form submit hook
function fetchit7_param_form_force_triggering_element($form, &$form_state) {
	/*
	 * the triggering element variable gets set based on which button was pressed
	 * the idea is that it is set (has a value) right after the button is pressed,
	 *   but NOT later (in the drupal form processing) when this function seems to get called again
	 *   so checking with isset() is a way to add some code to fire RIGHT AFTER an all or nothing button is pressed
	 *   but we want this function to do nothing on other calls
	 *   the code we want to add should save in a drupal variable which button was pressed
	 *     and clear out all the other button press saves so only one is ever set at a time
	 *     then when the form is drawn it can look for this variable and set the default values accordingly
	 *     also, we will have to save any changes the user has made to other form elements
	 *     and restore them when the form is redrawn.
	 *     this means essentially that EVERY form element will need to have it's state saved
	 *     so it can be restored when the form is redrawn.  the one form element associated with the all or none
	 *     button pressed will not save it's current state, but will save either a all or none state
	 *     that way, the form redrawing code does not have to be very smart
	 */
	if(isset($form_state['triggering_element'])) {
		$triggering_element_name = $form_state['triggering_element']['#name'];
		//debug($triggering_element_name,microtime(true).' fetchit7_param_form_force_triggering_element: a triggering_element is set; the name is:');
		// blow up the element name to get the info out of it
		$triggering_element_name_components = array();
		$triggering_element_name_components = explode('_',$triggering_element_name);
		//debug($triggering_element_name_components,microtime(true).' fetchit7_param_form_force_triggering_element: element name components:');
		if ($triggering_element_name_components[0]=="fetchit7") {
			//save the existing state of ALL the form elements
			$param_count = $form['param_count']['#value'];
			for ($param_ndx = 0; $param_ndx < $param_count; $param_ndx++) {
				$element_name = $triggering_element_name_components[0].'_'.$triggering_element_name_components[1].'_'.$triggering_element_name_components[2].'_'.$triggering_element_name_components[3].'_'.$param_ndx;
				//debug($element_name,microtime(true).' fetchit7_param_form_element: element name:');
				variable_set($element_name.'_set',true);
				variable_set($element_name.'_value',$form_state['values']['elements'][$element_name]);
				//debug($form_state['values']['elements'][$element_name],microtime(true).' fetchit7_param_form_element: element value:');
				// set them all to false, then set the triggering to true afterwards
				// notice that $element_name (the value/list control) is the base of $triggering_element_name (the all or none button)
				variable_set($element_name.'_allbtn',false);
				variable_set($element_name.'_nonebtn',false);
			}
			// create an internal drupal variable with the same name as the element, give it a boolean value of true
			// this is used to let the subsequent (re)render of the form know that it is redrawing
			//   after an all or none button press, because things happen differently in those cases
			variable_set($triggering_element_name,true);
			
			// what is this for?
			$form_state['triggering_element'] = $form['elements'][$form_state['triggering_element']['#name']];
		}
	} else {
		//debug('',microtime(true).' fetchit7_param_form_force_triggering_element: a triggering_element is NOT set');
	}
	/*
	if (isset($form_state['input']['all'])) {
		$string=$form_state['input']['all'];
		$form_state['triggering_element'] = $form['all'];
	    debug($string,'Hello from the fetchit7_param_form_force_triggering_element function. From here it appears all was pressed:');
	} elseif (isset($form_state['input']['none'])) {
		$string=$form_state['input']['none'];
		$form_state['triggering_element'] = $form['none'];
	    debug($string,'Hello from the fetchit7_param_form_force_triggering_element function. From here it appears none was pressed:');
	} elseif (isset($form_state['input']['actions']['submit'])) {
		$string=$form_state['input']['actions']['submit'];
		$form_state['triggering_element'] = $form['actions']['submit'];
	    debug($string,'Hello from the fetchit7_param_form_force_triggering_element function. From here it appears submit was pressed:');
	} else {
		if (isset($form_state['triggering_element'])) {
			$string=$form_state['triggering_element'];
			debug($string,'Hello from the fetchit7_param_form_force_triggering_element function. From here it appears nothing was pressed (yet?) but triggering element is:');
		} else {
			$string=null;
			debug($string,'Hello from the fetchit7_param_form_force_triggering_element function. From here it appears nothing was pressed (yet?) but triggering element is:');
		}
	}
	*/
	return $form;
}
/**
 * Process form validation
 */
function fetchit7_param_all_validate($form, &$form_state) {
	//debug($form_state['triggering_element']['#name'],microtime(true).' fetchit7_param_all_validate function: triggering element:');
	//debug(variable_get($form_state['triggering_element']['#name'],FALSE),microtime(true).' fetchit7_param_all_validate function. triggering element variable value:');
	//drupal_set_message(t('Successfully validated all button.'), 'status');
	return;
}

function fetchit7_param_none_validate($form, &$form_state) {
	//debug($form_state['triggering_element']['#name'],microtime(true).' fetchit7_param_none_validate function: triggering element:');
	//debug(variable_get($form_state['triggering_element']['#name'],FALSE),microtime(true).' fetchit7_param_none_validate function: triggering element variable value:');
	//drupal_set_message(t('Successfully validated none button.'), 'status');
	return;
}

function fetchit7_param_form_validate($form, &$form_state) {
	//$string = $form['param_count']['#value'].' or '.$form_state['values']['param_count'];
	//debug($string,microtime(true).' fetchit7_param_form_validate function. $form[] param_count or $form_state[]:');
	$nid = $form['nid']['#value'];
	//$row_ndx = 0;
	//$param_ndx = 0;
	//$element_name = fetchit_make_element_name($nid,$row_ndx,$param_ndx);
	//debug($element_name,'$element_name');
	//$control_type = $form['elements'][$element_name]['#type'];
	//debug($control_type,'$control_type');
	//debug($form_state['values']['elements'],'$form_state[values][elements]');
}

function fetchit7_param_all_submit($form, &$form_state) {
	//debug($form_state['triggering_element']['#name'],microtime(true).' fetchit7_param_all_submit function: triggering element:');
	//debug(variable_get($form_state['triggering_element']['#name'],FALSE),microtime(true).' fetchit7_param_all_submit function: triggering element variable value:');
	//drupal_set_message(t('Successfully submitted all button.'), 'status');
}

function fetchit7_param_none_submit($form, &$form_state) {
	//debug($form_state['triggering_element']['#name'],microtime(true).' fetchit7_param_none_submit function: triggering element:');
	//debug(variable_get($form_state['triggering_element']['#name'],FALSE),microtime(true).' fetchit7_param_none_submit function: triggering element variable value:');
	//drupal_set_message(t('Successfully submitted none button.'), 'status');
}

/**
 * Process form submission
 */
function fetchit7_param_form_submit($form, &$form_state) {
  $nid = $form['nid']['#value'];
  $db_type = $form['db_type']['#value'];
  $db_handle = $form['db_handle']['#value'];
  $param_count = $form['param_count']['#value'];
  for ($param_ndx = 0; $param_ndx < $param_count; $param_ndx++) {
    $param_data_table = $form['param_data_table'][$param_ndx]['#value'];
    $param_data_ndx = $form['param_data_ndx'][$param_ndx]['#value'];
    $param_data_uid = $form['param_data_uid'][$param_ndx]['#value'];
    $param_type = $form['param_types'][$param_ndx]['#value'];
    //clear out the existing records
    $result = fetchit_remove_records($db_handle, $db_type, $param_data_table, $param_data_uid);
    //add the new record(s)
    $fld_names = $param_data_ndx;
    if (strlen($param_data_uid)) {
      $fld_names .= ',' . $param_data_uid;
      global $user;
      $user_uid = $user -> uid;
    }
    $query = "INSERT INTO $param_data_table ( $fld_names ) VALUES (";
    // here is the tricky part
    // depending on what kind of control it is, there could be one or multiple or no values to store
    $row_ndx = 0;
    $element_name = fetchit_make_element_name($nid, $row_ndx, $param_ndx);
    $control_type = $form['elements'][$element_name]['#type'];
    switch ($control_type) {
      case 'date':
        // the weird drupal core date element - an array of three values
        $tmp_val_str = $form_state['values']['elements'][$element_name]['year'];
        $tmp_val_str .= "-";
        $tmp_val_str .= $form_state['values']['elements'][$element_name]['month'];
        $tmp_val_str .= "-";
        $tmp_val_str .= $form_state['values']['elements'][$element_name]['day'];
        $val_str = fetchit_make_val_str($tmp_val_str, $form['field_types'][$param_ndx]['#value']);
        if (strlen($param_data_uid))
          $val_str .= "," . fetchit_make_val_str($user_uid, "integer");
        $query .= "$val_str)";
        $result = fetchit_run_query($db_type, $db_handle, $query);
        break;
      case 'select':
        $multiple = $form['elements'][$element_name]['#multiple'];
        if ($multiple) {
          $values = $form_state['values']['elements'][$element_name];
          foreach ($values as $value) {
            $val_str = fetchit_make_val_str($value, $form['field_types'][$param_ndx]['#value']);
            if (strlen($param_data_uid))
              $val_str .= "," . fetchit_make_val_str($user_uid, "integer");
            $qry = $query . $val_str . ")";
            $result = fetchit_run_query($db_type, $db_handle, $qry);
          }
        }
        else {
          $value = $form_state['values']['elements'][$element_name];
          $val_str = fetchit_make_val_str($value, $form['field_types'][$param_ndx]['#value']);
          if (strlen($param_data_uid))
            $val_str .= "," . fetchit_make_val_str($user_uid, "integer");
          $query .= "$val_str)";
          $result = fetchit_run_query($db_type, $db_handle, $query);
        }
        break;
      case 'checkboxes':
        $values = array_filter($form_state['values']['elements'][$element_name]);
        foreach ($values as $value) {
          $val_str = fetchit_make_val_str($value, $form['field_types'][$param_ndx]['#value']);
          if (strlen($param_data_uid))
            $val_str .= "," . fetchit_make_val_str($user_uid, "integer");
          $qry = $query . $val_str . ")";
          $result = fetchit_run_query($db_type, $db_handle, $qry);
        }
        break;
      default:
        //everything else
        $tmp_val_str = $form_state['values']['elements'][$element_name];
        //debug($tmp_val_str,'$tmp_val_str');
        //if (!strlen(trim($tmp_val_str))) break;
        $val_str = fetchit_make_val_str($tmp_val_str, $form['field_types'][$param_ndx]['#value']);
        //debug($val_str,'$val_str');
        if (strlen($param_data_uid))
          $val_str .= "," . fetchit_make_val_str($user_uid, "integer");
        //debug($val_str,'$val_str');
        $query .= "$val_str)";
        //debug($query,'$query');
        $result = fetchit_run_query($db_type, $db_handle, $query);
    }
  }

  $postqry = trim(fetchit7_content_field_fetcher($nid, 'post_function', 'value'));
  if (isset($postqry)) {
    if (strlen($postqry)) {
      if (strtolower(substr($postqry, 0, 4)) == 'php.') {
        $thefunction = substr($postqry, 4);
        global $user;
        $user_uid = (int)$user -> uid;
        $result = $thefunction($user_uid);
      }
      else {
        $result = fetchit_run_query($db_type, $db_handle, $postqry);
      }
      //TODO: check post function result?
    }
  }

  $redirect = trim(fetchit7_content_field_fetcher($nid, 'redirect', 'value'));
  if (isset($redirect)) {
    if (strlen($redirect)) {
      $form_state['redirect'] = $redirect;
    }
  }
  drupal_set_message(t('Successfully submitted changes.'), 'status');
}

function theme_fetchit7_param_form($variables) {
  $form = $variables['form'];
  $nid = $form['nid']['#value'];
  $form_id = $form['form_id']['#value'];
  //debug($form_id, microtime(true).' theme_fetchit7_param_form: $form_id:');
  //the drupal way is to do all the non display related preprocessing in a theme preprocessor function
  // ... move it there later ...
  $table_row_count = $form['table_rows']['#value'];
  //debug($table_row_count,'$table_row_count');
  $table_weight = $form['table_weight']['#value'];
  $submit_button_flag = $form['submit_button_flag']['#value'];
  //debug($table_weight,'$table_weight');
  //figure out the formatting
  $output = '<div class="' . $form_id . '_params">';
  //$output  = '';

  if ($table_row_count) {
    //figure out the table structure
    $param_count = $form['param_count']['#value'];
    //debug($param_count,'$param_count');
    $table_column_count = intval(($param_count - 1) / $table_row_count) + 1;
    $table_headers = $form['table_headers']['#value'];
    $table_headers_count = count($table_headers);
    $header = array();
    for ($col_ndx = 0; $col_ndx < $table_column_count; $col_ndx++) {
      if ($col_ndx < $table_headers_count) {
        $header[] = $table_headers[$col_ndx];
      }
      else {
        $header[] = '';
      }
    }
    $rows = array();
    for ($row_ndx = 0; $row_ndx < $table_row_count; $row_ndx++) {
      //debug($row_ndx,'$row_ndx');
      $row = array();
      for ($col_ndx = 0; $col_ndx < $table_column_count; $col_ndx++) {
        //debug($col_ndx,'$col_ndx');
      	// numbering goes down the columns first
        $p_ndx = $row_ndx + $col_ndx * $table_row_count;
        //debug($p_ndx,'$p_ndx');
        if ($p_ndx < $param_count) {
          $element_name = fetchit_make_element_name($nid, 0, $p_ndx);
          //debug($element_name,'$element_name');
          if ($submit_button_flag) {
          	if (isset($form['elements'][$element_name.'_allbtn']) && isset($form['elements'][$element_name.'_nonebtn'])) {
                //debug($form['param_allnone'][$p_ndx]['#value'],'$form[param_allnone][$p_ndx][#value]');
          		switch ($form['param_allnone'][$p_ndx]['#value']) {
	          		case '-':
		          		$row[] = drupal_render($form['elements'][$element_name.'_allbtn']).
		        	    			drupal_render($form['elements'][$element_name.'_nonebtn']).
		        	    			drupal_render($form['elements'][$element_name]);
	          			break;
	          		case '+':
	          		case '*':
		          		$row[] = drupal_render($form['elements'][$element_name]).
		    	        			drupal_render($form['elements'][$element_name.'_allbtn']).
		        	    			drupal_render($form['elements'][$element_name.'_nonebtn']);
	          			break;
	          		default:
		          		$row[] = drupal_render($form['elements'][$element_name]);
          	    }
          	} else {
          		$row[] = drupal_render($form['elements'][$element_name]);
          	}    
          } else {
          	$row[] = drupal_render($form['elements'][$element_name]);
          }
        } else {
          $sillyform[$p_ndx] = array(
          		'#type' => 'item',
          		'#title' => 'Empty Cell',
          		'#markup' => "I am an empty cell.", );
          $row[] = drupal_render($sillyform[$p_ndx]);
        }
      }
      $rows[] = $row;
    }
    //debug($table_weight,'$table_weight');
    $form['table'] = array(
    		'#theme' => 'table',
    		'#header' => $header,
    		'#rows' => $rows,
    		'#weight' => $table_weight,
    		'#empty' => "Empty table.", );
    $output .= drupal_render($form['table']);
  } else {
    $param_count = $form['param_count']['#value'];
	for ($p_ndx=0; $p_ndx < $param_count; $p_ndx++) {
	  $element_name = fetchit_make_element_name($nid, 0, $p_ndx);
	  if ($submit_button_flag) {
       	if (isset($form['elements'][$element_name.'_allbtn']) && isset($form['elements'][$element_name.'_nonebtn'])) {
       		switch ($form['param_allnone'][$p_ndx]['#value']) {
       			case '-':
	  				$output .= drupal_render($form['elements'][$element_name.'_allbtn']);
	  				$output .= drupal_render($form['elements'][$element_name.'_nonebtn']);
	  				$output .= drupal_render($form['elements'][$element_name]);
       				break;
       			case '+':
       			case '*':
       				$output .= drupal_render($form['elements'][$element_name]);
       				$output .= drupal_render($form['elements'][$element_name.'_allbtn']);
       				$output .= drupal_render($form['elements'][$element_name.'_nonebtn']);
       				break;
       			default:
       				$output .= drupal_render($form['elements'][$element_name]);
       		}
       	} else {
       		$output .= drupal_render($form['elements'][$element_name]);
       	}
	  } else {
	  	$output .= drupal_render($form['elements'][$element_name]);
	  }
	}
  }
  $output .= '</div>';
  //$submit_button_flag = $form['submit_button_flag']['#value'];
  if ($submit_button_flag) {
    $output .= '<div class="' . $form_id . '_submit">';
    $output .= drupal_render($form['actions']['submit']);
    $output .= '</div>';
  }
  $addbody_flag = $form['addbody_flag']['#value'];
  if ($addbody_flag) {
    $output .= '<div class="' . $form_id . '_body">';
    $output .= drupal_render($form['body']);
    $output .= '</div>';
  }
  $output .= '<div class="' . $form_id . '_other">';
  $output .= drupal_render_children($form);
  $output .= '</div>';

  return $output;
}

//===================================
// fetchit7 editable grid form content
//===================================

/**
 * Form builder.  Setup the elements (controls) to be displayed on the fetchit7 editable grid form
 *
 * @ingroup forms
 * @see system_settings_form().
 *
 * this function creates the editable grid form
 * first it gets the data stored in the drupal node and content type tables (created in the .install file)
 * that define what the form will look like
 * this is done via a function, fetchit7_content_field_fetcher(), that calls the drupal API functions for extracting node and field data
 * next it stores that data in a special array, $user_field_data, that was designed when fetchit2 was created
 * using this array maximizes the amount of fetchit2 code that can be reused
 * that array is used with a special form builder function, fetchit7_param_element_form,
 * that calls a bunch of element (form controls) creation functions
 * this collection of form elements is pre-rendered and then arranged on the form
 * that the user uses to enter the various parameter data or make parameter value selections
 *
 */
function fetchit7_egrid_form($form, &$form_state) {
  $addbody_flag = TRUE; // until we have a flag for this it on the fetchit7 form
  // some things we can put in the $form
  //$form['#attached']['css'] = array(drupal_get_path('module', 'block') . '/block.css');
  $form['elements'] = array();
  $form['#tree'] = TRUE;

  // get nid embedded into original form_id; format fetchit7_param_form_X### - nid starts at character 21
  $form_id = $form_state['build_info']['form_id'];
  // to get here, this is definitely set
  //debug('fetchit7_egrid_form 1');
  $nid = (int) substr($form_id, 21);
  $form['nid'] = array('#type' => 'value', '#value' => $nid);

  // get database type and store it (hidden) in the form so it is available to the form validate and submit functions
  // this is not optional, and can not be filled in with a default, so if it is missing, throw an error and exit
  // this should be a single string
  $db_type = trim(fetchit7_content_field_fetcher($nid, 'db_type', 'value'));
  if (strlen($db_type)) {
    $form['db_type'] = array('#type' => 'value', '#value' => $db_type);
  }
  else {
    drupal_set_message("An error occurred in fetchit7_egrid_form (nid = $nid) when accessing the database type.", 'error');
    return $form;
  }

  // get database handle
  if ($db_handle = fetchit_get_db_handle($nid)) {
    $form['db_handle'] = array('#type' => 'value', '#value' => $db_handle);
    //debug($db_handle,'$db_handle');
  }
  else {
    drupal_set_message("An error occurred in fetchit7_egrid_form (nid = $nid) when creating the database connection handle.", 'error');
    return $form;
  }

  // run the pre query before doing anything else
  // TODO: figure out why pre-fn is getting hit twice prior to the form drawing!!!
  // get query string. this is optional, so if it is missing, move on
  // this should be a single string
  $preqry = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_pre_function', 'value'));
  if (strlen($preqry)) {
    if (strtolower(substr($preqry, 0, 4)) == 'php.') {
      $thefunction = substr($preqry, 4);
      global $user;
      $user_uid = (int)$user -> uid;
      $result = $thefunction($user_uid);
    }
    else {
      $result = fetchit_run_query($db_type, $db_handle, $preqry);
    }
    //TODO: check pre function result?
  }

  // get database source data view/table name - this is required - can not be missing
  $view_name = trim(fetchit7_content_field_fetcher($nid, 'source_view', 'value'));
  if (strlen($view_name)) {
    $form['view_name'] = array('#type' => 'value', '#value' => trim($view_name));
  }
  else {
    drupal_set_message("An error occurred in fetchit7_egrid_form (nid = $nid) when accessing the source data view/table name.", 'error');
    return $form;
  }

  // get drupal user id (uid) field name
  // IMPORTANT: knowing the uid field is critical to the query,
  //		but unlike parameter input forms, it is not used to control display
  //		that will be up to the user to decide using the enabled and hidden fields
  $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
  $form['user_uid_field'] = array('#type' => 'value', '#value' => $user_uid_field);

  // get key field name
  // IMPORTANT: knowing the key field can be critical for some functions,
  //		but unlike parameter input forms, it is not used to control display
  //		that will be up to the user to decide using the enabled and hidden fields
  $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
  $form['key_field'] = array('#type' => 'value', '#value' => $key_field);

  // parameter table weight should be a single integer value
  $table_weight = fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_weight', 'value');
  $form['table_weight'] = array('#type' => 'value', '#value' => $table_weight);

  // table attributes
  // appendrecord should be a single boolean value
  // fetchit7_field_egrid_append
  $appendrecord = fetchit7_content_field_fetcher($nid, 'fetchit7_field_egrid_append', 'value');
  $form['appendrecord'] = array('#type' => 'value', '#value' => $appendrecord);

  // parameter table_rows should be a single integer value
  // NOTE: this is a ambiguous idea on the editable grid, so either
  //    1. don't collect this value
  //		2. use it as a page size on a paged table display
  //		3. use it as a maximum table size, by restricting the records returned in the view (IMPLEMENTED)
  $table_rows = fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_tbl_rows', 'value');
  $form['table_rows'] = array('#type' => 'value', '#value' => $table_rows);

  // table_headers should be an array of text values
  // warning: d7 does not return empty strings at all
  // but if someone put spaces as a placeholder, trim them to an empty string
  // IMPORTANT: apply these in order to the table columns AS DISPLAYED (i.e. skipping hidden columns)
  $table_headers = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_table_headers', 'values'));
  //should return an array
  $form['table_headers'] = array('#type' => 'value', '#value' => $table_headers);

  // addbody
  // addbody_flag should be a single boolean value
  // use it if something valid is entered.   if not set it to a default of true
  //$addbody_flag = fetchit7_content_field_fetcher($nid, 'addbody_flag', 'value');
  $form['addbody_flag'] = array('#type' => 'value', '#value' => $addbody_flag);

  // submit button attributes
  // submit button label should be a single string value
  // use it if something is entered.   if not offer a default of Submit
  $submit_label = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_label', 'value'));
  if (strlen($submit_label)) {
    $submit_flag = TRUE;
  }
  else {
    $submit_flag = TRUE;
    $submit_label = 'Submit';
    // in case we HAVE to have a button
  }
  $form['submit_label'] = array('#type' => 'value', '#value' => $submit_label);

  // submit button weight should be a single integer value
  // use it if something is entered.   if not offer a default of 10 - puts it at or close to the bottom of the form
  $submit_weight = fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_weight', 'value');
  $form['submit_weight'] = array('#type' => 'value', '#value' => $submit_weight);

  // redirectl should be a single string value
  // optional, use it if something is entered.   if not set it to an empty string
  $submit_redirect = trim(fetchit7_content_field_fetcher($nid, 'fetchit7_field_submit_redirect', 'value'));
  $form['submit_redirect'] = array('#type' => 'value', '#value' => $submit_redirect);

  // table attributes
  // showtitles should be a single boolean value
  $showtitles = fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_shw_ttl', 'value');
  $form['showtitles'] = array('#type' => 'value', '#value' => $showtitles);

  // get the source data into a data object
  if ($table_rows) {
    if (!($db_data_object = fetchit_get_data_object($db_type, $db_handle, $view_name, $user_uid_field, $table_rows))) {
      drupal_set_message("An error occurred in fetchit7_egrid_form (nid = $nid) when querying the source data table.", 'error');
      return $form;
    }
  }
  else {
    if (!($db_data_object = fetchit_get_data_object($db_type, $db_handle, $view_name, $user_uid_field, 1))) {//just get one row since it will not be shown in a table
      drupal_set_message("An error occurred in fetchit7_egrid_form (nid = $nid) when querying the source data table.", 'error');
      return $form;
    }
  }
  //debug('fetchit7_egrid_form 2');

  // get the source table meta data
  $row_count = fetchit_get_row_count($db_type, $db_data_object);
  $form['row_count'] = array('#type' => 'value', '#value' => $row_count);
  $field_count = fetchit_get_field_count($db_type, $db_data_object);
  $form['field_count'] = array('#type' => 'value', '#value' => $field_count);
  $field_names = array();
  if (!($field_names = fetchit_get_field_names($db_type, $db_data_object))) {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field names.", 'error');
    return $form;
  }
  $form['field_names'] = array('#type' => 'value', '#value' => $field_names);
  $field_types = array();
  if (!($field_types = fetchit_get_field_types($db_type, $db_data_object))) {
    drupal_set_message("An error occurred in fetchit7_table_form (nid = $nid) when accessing the source data table field types.", 'error');
    return $form;
  }
  $form['field_types'] = array('#type' => 'value', '#value' => $field_types);

  // check that the key and uid fields are in the table
  // no key field means no delete query
  $key_field_found = FALSE;
  $key_field_index = 0;
  $uid_field_found = FALSE;
  $uid_field_index = 0;
  for ($field_index = 0; $field_index < $field_count; $field_index++) {
    if ($field_names[$field_index] == $key_field) {
      $key_field_found = TRUE;
      $key_field_index = $field_index;
    }
    if ($field_names[$field_index] == $user_uid_field) {
      $uid_field_found = TRUE;
      $uid_field_index = $field_index;
    }
  }
  $form['key_field_found'] = array('#type' => 'value', '#value' => $key_field_found);
  $form['key_field_index'] = array('#type' => 'value', '#value' => $key_field_index);
  $form['uid_field_found'] = array('#type' => 'value', '#value' => $uid_field_found);
  $form['uid_field_index'] = array('#type' => 'value', '#value' => $uid_field_index);

  //debug('fetchit7_egrid_form 3');
  
  // figure out if we are adding a new row
  //debug($appendrecord.', '.$row_count.', '.$table_rows,'$appendrecord $row_count $table_rows');
  if (!$row_count) $appendrecord=TRUE;
  if ($appendrecord) {
  	$form['appendrecord'] = array('#type' => 'value', '#value' => $appendrecord);
    $itsarealrecord = FALSE;
    $addanewrow = TRUE;
    $row_count = 1;
	  $form['row_count'] = array('#type' => 'value', '#value' => $row_count);
		if ($table_rows) {
			 $table_rows = 1;
		}
    $existdefaults = FALSE;
    //$defaults_table = trim(fetchit_get_cck_val(F_DR_CONTENT_DEFAULTS_TABLE_TBL,F_DR_CONTENT_DEFAULTS_TABLE_FLD,F_DR_NID_FLD,$nid))
    $defaults_table = '';
    if (strlen($defaults_table)) {
      // note the assumption that the user id field in the defaults table is the same as in the data table
      // and that we are appending only one record, so only one record of defaults is needed
      $db_defaultdata_object = fetchit_get_data_object($err, $db_type, $db_handle, $defaults_table, $user_uid_field, 1);
      $default_row_count = fetchit_get_row_count($err, $db_type, $db_defaultdata_object);
      if ($default_row_count)
        $existdefaults = TRUE;
    }
  } else {
	 	$form['appendrecord'] = array('#type' => 'value', '#value' => $appendrecord);
	  $itsarealrecord = TRUE;
	  $addanewrow = FALSE;
  }
  

  // now load any individual field settings that the user may have entered
  //
  // fetchit7_content_field_fetcher() returns the type of the underlying field,
  // So these should all be arrays of string values entered by the user
  // They are arrays therefore pass the values format tag to fetchit7_content_field_fetcher()
  // They are strings, so there will need to do some type conversion later
  // Depending on what the user entered, the arrays are potentially different sizes
  // warning: d7 does not return empty strings at all
  // These are preprocessed (cleaned up and converted to the appropriate variable type)
  // and stored in the $user_field_data array below
  // check that they are defined and are arrays.  some can not be empty...

  // the param names here should be field names from the source table
  // note that the key and uid fields can be skipped since they are not displayed
  $param_names_raw = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_name', 'values'));
  //should return an array
  $param_names = array();
  $foundone = FALSE;
  foreach ($param_names_raw as $param_name) {
    if (strlen($param_name)) {
      $param_names[] = $param_name;
      $foundone = TRUE;
    }
  }
  //if (!$foundone) {
  //  drupal_set_message("An error occurred creating the parameter input form (nid = $nid).  No parameter names were specified.",'error');
  //  return $form;
  //}
  // optional
  $param_titles = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_title', 'values'));
  //should return an array
  $param_descriptions = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_desc', 'values'));
  //should return an array
  // i equivocate on this, but for now is optional - there is a default control type that is picked based on the output field type
  $param_types = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_type', 'values'));
  //should return an array
  // weights are integers, but leave these input values as strings until later
  $param_weights = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_weight', 'values'));
  //should return an array
  // boolean, but leave these input values as strings until later, also can deal with default values then
  $param_enabled = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_enabled', 'values'));
  //should return an array
  // boolean, but leave these input values as strings until later, also can deal with default values then
  $param_hidden = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_hidden', 'values'));
	//debug($param_hidden,'$param_hidden');
  //should return an array
  // boolean, but leave these input values as strings until later, also can deal with default values then
  $param_required = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_required', 'values'));
  //should return an array
  // optional
  $param_str = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_parstr', 'values'));
  //should return an array

  $param_list_tbl = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_tbl', 'values'));
  //should return an array
  $param_list_key = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_key', 'values'));
  //should return an array
  $param_list_dsp = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_dsp', 'values'));
  //should return an array
  $param_list_uid = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_uid', 'values'));
  //should return an array
  $param_list_def = array_map('trim', fetchit7_content_field_fetcher($nid, 'fetchit7_field_par_fld_list_def', 'values'));
  //should return an array

  // these are not needed for an editable grid
  //$param_data_tbl = array_map('trim',fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_tbl','values')); //should return an array
  //$param_data_key = array_map('trim',fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_key','values')); //should return an array
  //$param_data_uid = array_map('trim',fetchit7_content_field_fetcher($nid,'fetchit7_field_par_fld_data_uid','values')); //should return an array
  //debug('fetchit7_egrid_form 4');
  
  //use the above arrays to contruct the $user_field_data array
  // note that in this editable grid formm the source table defines the list and count of fields, not the param list
  // we will use defaults is possible where items are missing
  // exit gracefully with error message otherwise
  $control_list = fetchit7_get_control_list();
  $control_list_string = fetchit7_get_control_list_string();
  $user_field_data = array();
  $user_control = array();

  for ($field_index = 0; $field_index < $field_count; $field_index++) {
    $param_found = 0;
    $param_index = 0;
    foreach ($param_names as $param_name) {// they will NOT be blank at this point
      if ($param_name == $field_names[$field_index]) {
        $param_found = $param_index + 1;
      }
      ++$param_index;
    }

    $user_field_data[$field_index][F_DR_FIELD_NAME_STR] = $field_names[$field_index];

    // title - default of field name is OK
    if ($param_found) {
      if (($param_found - 1) < count($param_titles)) {
        $user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = $param_titles[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = $field_names[$field_index];
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_TITLE_STR] = $field_names[$field_index];
    }

    //description - default of '' is OK
    if ($param_found) {
      if (($param_found - 1) < count($param_descriptions)) {
        $user_field_data[$field_index][F_DR_FIELD_DESC_STR] = $param_descriptions[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DESC_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DESC_STR] = '';
    }

    // control types - require some more checking since they are typed into a text box
    // default is 0, and user_control() = FALSE, which means let the field type determine the control element type
    // (the code should have an appropriate default control element type specified for each possible database field type)
    if ($param_found) {
      if (($param_found - 1) < count($param_types)) {
        // see if the entered control type matches something we know about
        if ($key = array_search(strtolower($param_types[$param_found - 1]), $control_list)) {
          $user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = $key - 1;
          // the fetchit2 code wants an index here, and we had incremented these by 1 above to make array_search() easier to use
          $user_control[$field_index] = TRUE;
        }
        else {
          $user_control[$field_index] = FALSE;
          $user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = -99; //this unknown code should roll into a default type
        }
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = -99; //this unknown code should roll into a default type
        $user_control[$field_index] = FALSE;
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_INPUT_TYPE_STR] = -99; //this unknown code should roll into a default type
      $user_control[$field_index] = FALSE;
    }

    //weight default of field_index is OK (defaults to the order in the table)
    if ($param_found) {
      if (($param_found - 1) < count($param_weights)) {
        $user_field_data[$field_index][FETCHIT7_FIELD_WEIGHT_STR] = (int)$param_weights[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_WEIGHT_STR] = $field_index;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_WEIGHT_STR] = $field_index;
    }

    //$param_enabled
    // fetchit2 uses disabled - so default of false means it should be enabled (editable)
    if ($param_found) {
      if (($param_found - 1) < count($param_enabled)) {
        //$enabled = filter_var($param_enabled[$param_found - 1], FILTER_VALIDATE_BOOLEAN);
        $enabled = fetchit_booleantointeger($param_enabled[$param_found - 1]);
				if($enabled) {
					$user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = 0;
				} else {
					$user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = 1;
				}
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = 0;
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DISABLED_STR] = 0;
    }

    //$param_hidden
    // default of false means it should be displayed
    if ($param_found) {
      if (($param_found - 1) < count($param_hidden)) {
        $user_field_data[$field_index][FETCHIT7_FIELD_HIDDEN_STR] = 
        	fetchit_booleantointeger($param_hidden[$param_found - 1]);
        //filter_var($param_hidden[$param_found - 1], FILTER_VALIDATE_BOOLEAN);
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_HIDDEN_STR] = 0;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_HIDDEN_STR] = 0;
    }
		//debug($field_index,'$field_index');
		//debug(FETCHIT7_FIELD_HIDDEN_STR,'FETCHIT7_FIELD_HIDDEN_STR');
		//debug($user_field_data[$field_index][FETCHIT7_FIELD_HIDDEN_STR],'$user_field_data');

    //$param_required
    //default of required = true is OK
    if ($param_found) {
      if (($param_found - 1) < count($param_required)) {
        $user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] =
        	fetchit_booleantointeger($param_required[$param_found - 1]);
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = 1;
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_REQUIRED_STR] = 1;
    }

    //$param_str parameter string
    //title default of '' is OK
    if ($param_found) {
      if (($param_found - 1) < count($param_str)) {
        $user_field_data[$field_index][F_DR_FIELD_PARAMETER_STR] = $param_str[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_PARAMETER_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_PARAMETER_STR] = '';
    }

    //$param_list_tbl F_DR_FIELD_LIST_TBL_STR
    //table name default of '' is OK
    if ($param_found) {
      if (($param_found - 1) < count($param_list_tbl)) {
        $user_field_data[$field_index][F_DR_FIELD_LIST_TBL_STR] = $param_list_tbl[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_LIST_TBL_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_LIST_TBL_STR] = '';
    }

    //$param_list_key - not optional if a list table is specified, so don't put a blank value as a default
    if ($param_found) {
      if (($param_found - 1) < count($param_list_key)) {
        if (strlen($param_list_key[$param_found - 1])) {
          $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = $param_list_key[$param_found - 1];
        }
        else {
          $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = F_DR_LIST_TBL_NDX_FLD;
        }
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = F_DR_LIST_TBL_NDX_FLD;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_LIST_NDX_STR] = F_DR_LIST_TBL_NDX_FLD;
    }

    //$param_list_dsp - not optional if a list table is specified, so don't put a blank value as a default
    if ($param_found) {
      if (($param_found - 1) < count($param_list_dsp)) {
        if (strlen($param_list_dsp[$param_found - 1])) {
          $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = $param_list_dsp[$param_found - 1];
        }
        else {
          $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = F_DR_LIST_TBL_DSP_FLD;
        }
      }
      else {
        $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = F_DR_LIST_TBL_DSP_FLD;
      }
    }
    else {
      $user_field_data[$field_index][FETCHIT7_FIELD_LIST_DSP_STR] = F_DR_LIST_TBL_DSP_FLD;
    }

    //$param_list_uid F_DR_FIELD_LIST_UID_STR
    // uid field name - blank of not used - default of blank is OK (usually the items in a parameter selection list are not user specific)
    if ($param_found) {
      if (($param_found - 1) < count($param_list_uid)) {
        $user_field_data[$field_index][F_DR_FIELD_LIST_UID_STR] = $param_list_uid[$param_found - 1];
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_LIST_UID_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_LIST_UID_STR] = '';
    }

    //$param_list_def F_DR_FIELD_DEFAULT_STR
    if ($param_found) {
      if (($param_found - 1) < count($param_list_def)) {
        if (strlen($param_list_def[$param_found - 1])) {
          $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = $param_list_def[$param_found - 1];
        }
        else {
          $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
        }
      }
      else {
        $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
      }
    }
    else {
      $user_field_data[$field_index][F_DR_FIELD_DEFAULT_STR] = '';
    }
  }

  //debug('fetchit7_egrid_form 5');

  // loop through the source table rows and fields
  // create renderable form control element constructions in the form() array for those values that will be displayed
	// the source table field values and field types plus the the user entered
	// values stored in user_field_data() will provide the information necessary to create the control elements
  // critical to the operation of this form are the unique form ids of the rendered control elements
  // the theme function will render them individually in a stack (table_rows=0) or in a HTML table (table_rows>0)
  for ($r = 0; $r < $row_count; $r++) {
    for ($i = 0; $i < $field_count; $i++) {
      $element_name = fetchit_make_element_name($nid, $r, $i);
     	$disabled = $user_field_data[$i][F_DR_FIELD_DISABLED_STR];
     	if ($disabled) {
     		$enabled = 0;
     	} else {
     		$enabled = 1;
     	}
     	$hidden = $user_field_data[$i][FETCHIT7_FIELD_HIDDEN_STR];
			//debug ($r.' '.$i.' '.$hidden, '$r $i $hidden');

      // set a default value to pass into the element generator
      // this value is used unless the user specified a default value in the configuration
      // get the last entered value from the selection table
      // depending on the control type, this is either a single value or an array of values...
      // but because this is an egrid, it must be an array containing a single value
      if ($itsarealrecord) {
	      $value = fetchit_get_data($db_type,$db_data_object,$r,$i);
				//if ($i==1) debug('reading from table: $value='.$value);
	      $form['sourcetable'][$r][$i] = array('#type' => 'value', '#value' => $value); // the whole table is stored here in case it is needed
	      switch ($user_field_data[$i][F_DR_FIELD_INPUT_TYPE_STR]) {
	        case F_DR_CONTROL_CHECKBOXES:
	        case F_DR_CONTROL_DROPDOWN_MULTI:
	          $default_value = array($value);
	          break;
	        case F_DR_CONTROL_DROPDOWN_SINGLE:
	        case F_DR_CONTROL_RADIOBTN:
	        case F_DR_CONTROL_CHECKBOX:
	          $default_value = $value;
	          break;
	        case F_DR_CONTROL_TEXTBOX:
	          $default_value = $value;
	          break;
	        case F_DR_CONTROL_DATESEL:
	        case F_DR_CONTROL_DATETIMESEL:
	        case F_DR_CONTROL_POPUPDATE:
	          $default_value = $value;
	          break;
	        default:
	          $default_value = $value;
	      }
      } else {
      	$default_value='';
      }
			//debug('field '.$i.' converted to default: $default_value='.$default_value);

      // add everything else to the form we might need in the theme, validate, or submit functions
      //$form['param_count'] = array('#type' => 'value','#value' => ($param_count+1)); // update this each time through
      //$form['param_indices'][$param_count] = array('#type' => 'value','#value' => $i);
      //$form['field_types'][$i] = array('#type' => 'value','#value' => $data_tbl_ndx_type);
      $form['param_names'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_NAME_STR]);
      $form['param_types'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_INPUT_TYPE_STR]);
      $form['param_titles'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_TITLE_STR]);
      $form['param_descriptions'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DESC_STR]);
      $form['param_weights'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][FETCHIT7_FIELD_WEIGHT_STR]);
      $form['param_disabled'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][F_DR_FIELD_DISABLED_STR]);
      $form['param_hidden'][$i] = array('#type' => 'value', '#value' => $user_field_data[$i][FETCHIT7_FIELD_HIDDEN_STR]);
      //$form['param_data_table'][$param_count] = array('#type' => 'value','#value' => $user_field_data[$i][F_DR_FIELD_DATA_TBL_STR]);
      //$form['param_data_ndx'][$param_count] = array('#type' => 'value','#value' => $user_field_data[$i][F_DR_FIELD_DATA_KEY_STR]);
      //$form['param_data_uid'][$param_count] = array('#type' => 'value','#value' => $user_field_data[$i][F_DR_FIELD_DATA_UID_STR]);

      // set the index that determines whether the user specified the control element type ($user_field_ndx>0)
      // or not ($user_field_ndx=0)
      // for egrid, because we set defaults above, this can be >0
      if ($user_control[$i]) {
        $user_field_ndx = $i + 1;
      }
      else {
        $user_field_ndx = $i + 1;
        //$user_field_ndx = 0;
      }

      // the following creates the "renderable array" of the next parameter input control (drupal form element)
      //   to be displayed on this form ($nid)
      // stack these up in the array. "How" they are to be displayed will be handled by the theme function
      $form['elements'][$element_name] = 
      	fetchit7_param_element_form(
      		$field_names[$i],
      		$field_types[$i], 
      		$db_handle, 
      		$user_field_ndx, 
      		$user_field_data, 
      		$nid, 
      		$showtitles, 
      		$default_value, 
      		$enabled, // 
					$hidden); // this is used to draw a special value type form element so that it's in the form 
  	}
  }
  //debug('fetchit7_egrid_form 6');

  $submit_button_flag = TRUE;
  $form['submit_button_flag'] = array('#type' => 'value', '#value' => $submit_button_flag);
  if ($submit_button_flag) {
    //$form['actions'] = array(
    //  '#tree' => FALSE,
    //  '#type' => 'actions',
    //);
    //debug($submit_weight,'$submit_weight');
    $form['actions']['submit'] = array(
    	'#type' => 'submit',
    	'#value' => $submit_label,
    	'#weight' => $submit_weight,
		);
  }
  //debug('fetchit7_egrid_form 7');

  return $form;
}

/**
 * Process form validation
 */
function fetchit7_egrid_form_validate($form, &$form_state) {
  $nid = $form['nid']['#value'];
  //debug($nid, 'fetchit7_egrid_form_validate $nid');
}

/**
 * Process form submission
 */
function fetchit7_egrid_form_submit($form, &$form_state) {
	// need these for the delete query
	$user_uid_field = $form['user_uid_field']['#value'];
	$db_type = $form['db_type']['#value'];
	$db_handle = $form['db_handle']['#value'];
	$table_name = $form['view_name']['#value'];
	$appendrecord = $form['appendrecord']['#value'];
	//debug(!$appendrecord,'!$appendrecord');
	if (!$appendrecord) {
		//clear out the existing records
		$result = fetchit_remove_records($db_handle,$db_type,$table_name,$user_uid_field);
	}

	$nid = $form['nid']['#value'];
	$field_count = $form['field_count']['#value'];
	$row_count = $form['row_count']['#value'];
	$field_names = $form['field_names']['#value'];
	$field_types = $form['field_types']['#value'];

	// create the list of fields for the insert query
	//  this is ALL the fields in the table, not just the ones displayed in the egrid
	for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
		if ($field_ndx) {
			$fld_names .= ",".$field_names[$field_ndx];
		} else {
			$fld_names = $field_names[$field_ndx];
		}
	}
	for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
		$query = "INSERT INTO $table_name($fld_names) VALUES (";
		$val_str = "";
		for ($field_ndx = 0; $field_ndx < $field_count; $field_ndx++) {
			$element_name = fetchit_make_element_name($nid,$row_ndx,$field_ndx);
			if ($field_ndx) {
				$val_str .= ",";
			}
			if ($field_names[$field_ndx] == $user_uid_field) {
				global $user;
				$user_uid = $user->uid;
				$val_str .= fetchit_make_val_str($user_uid,"integer");
			//} elseif ($field_ndx == ($key_field_ndx-1)) {
			//	$val_str .= fetchit_make_val_str($form['key_field'][$row_ndx]['#value'],"integer");
			} else {
				$control_type = $form['elements'][$element_name]['#type'];
				switch ($control_type) {
					case 'value':
						$tmp_val_str = $form_state['values']['elements'][$element_name];
						break;
					case 'date': // the weird drupal core date element - an array of three values
						$disabled = $form['elements'][$element_name]['#disabled']; 
						if ($disabled) {
							$tmp_val_str = $form['elements'][$element_name]['#default_value']['year'];
							$tmp_val_str .= "-";
							$tmp_val_str .= $form['elements'][$element_name]['#default_value']['month'];
							$tmp_val_str .= "-";
							$tmp_val_str .= $form['elements'][$element_name]['#default_value']['day'];
						} else {
							//debug($element_name,'$element_name');
							//debug($form_state,'$form_state');
							$tmp_val_str = $form_state['values']['elements'][$element_name]['year'];
							$tmp_val_str .= "-";
							$tmp_val_str .= $form_state['values']['elements'][$element_name]['month'];
							$tmp_val_str .= "-";
							$tmp_val_str .= $form_state['values']['elements'][$element_name]['day'];
						}
						break;
		      case 'select':
		        $multiple = $form['elements'][$element_name]['#multiple'];
		        if ($multiple) {
		          $values = array_filter($form_state['values']['elements'][$element_name]);
							if (count($values)) {
								$options_keys = array_keys($values);
								$tmp_val_str = $options_keys[0]; //only one record so only one value
							} else {
								// nothing selected
								$tmp_val_str = '';
							}
						}
		        else {
		          $tmp_val_str = $form_state['values']['elements'][$element_name];
		        }
		        break;
		      case 'checkboxes':
		        $values = array_filter($form_state['values']['elements'][$element_name]);
						if (count($values)) {
							$options_keys = array_keys($values);
							$tmp_val_str = $options_keys[0]; //only one record so only one value
						} else {
							// nothing selected
							$tmp_val_str = '';
						}
		        break;
					default: //everything else until we get another that is not a string
						$disabled = $form['elements'][$element_name]['#disabled']; 
						if ($disabled) {
							$tmp_val_str = $form['elements'][$element_name]['#default_value'];
						} else {
							$tmp_val_str = $form_state['values']['elements'][$element_name];
						}
				}
				$val_str .= fetchit_make_val_str($tmp_val_str,$field_types[$field_ndx]);
			}
		}
		$query .= "$val_str)";
		//drupal_set_message($query);
		$result = fetchit_run_query($db_type,$db_handle,$query);
	}

  $postqry = trim(fetchit7_content_field_fetcher($nid, 'post_function', 'value'));
  if (isset($postqry)) {
    if (strlen($postqry)) {
      if (strtolower(substr($postqry, 0, 4)) == 'php.') {
        $thefunction = substr($postqry, 4);
        global $user;
        $user_uid = (int)$user -> uid;
        $result = $thefunction($user_uid);
      }
      else {
        $result = fetchit_run_query($db_type, $db_handle, $postqry);
      }
		}
	}

  $redirect = trim(fetchit7_content_field_fetcher($nid, 'redirect', 'value'));
  if (isset($redirect)) {
    if (strlen($redirect)) {
      $form_state['redirect'] = $redirect;
    }
  }
	
  drupal_set_message(t('Successfully submitted changes.'), 'status');
}

function theme_fetchit7_egrid_form($variables) {
  //debug('theme_fetchit7_egrid_form 1');
  $form = $variables['form'];
  $nid = $form['nid']['#value'];
  $form_id = $form['form_id']['#value'];
  //the drupal way is to do all the non display related preprocessing in a theme preprocessor function ... move it there later ...
  $table_row_count = $form['table_rows']['#value'];
  //debug($table_row_count,'$table_row_count');
  $table_weight = $form['table_weight']['#value'];
  //debug($table_weight,'$table_weight');
  //figure out the formatting
  $output = '<div class="' . $form_id . '_egrid">';
  //$output  = '';

  if ($table_row_count) {
	  //debug('theme_fetchit7_egrid_form 2');
    //figure out the table structure
    $row_count = $form['row_count']['#value'];
    $field_count = $form['field_count']['#value'];
		$col_count = 0;
    $table_headers = $form['table_headers']['#value'];
    $table_headers_count = count($table_headers);
		if ($table_headers_count==1 && $table_headers[0]=='') $table_headers_count = 0; //in other words, assume the user did not enter anything
    $header = array();
    for ($fld_ndx = 0; $fld_ndx < $field_count; $fld_ndx++) {
			$hidden = fetchit_booleantointeger($form['param_hidden'][$fld_ndx]['#value']);
	    //debug($fld_ndx.' '.$theval.' '.$hidden,'$fld_ndx $theval $hidden');
    	if (!$hidden) { // hidden fields should have been created with a non-renderable control element type
	      if ($col_count < $table_headers_count) {
	        $header[] = $table_headers[$col_count];
	      }
	      else {
	        $header[] = $form['param_names'][$fld_ndx]['#value'];
	      }
				++$col_count;
    	}
    }
    $rows = array();
    for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
      $row = array();
	    for ($fld_ndx = 0; $fld_ndx < $field_count; $fld_ndx++) {
        $element_name = fetchit_make_element_name($nid, $row_ndx, $fld_ndx);
				$hidden = fetchit_booleantointeger($form['param_hidden'][$fld_ndx]['#value']);
	    	if (!$hidden) { // hidden fields should have been created with a non-renderable control element type
        	$row[] = drupal_render($form['elements'][$element_name]);
				}
   		}
      $rows[] = $row;
  	}
  	//debug($table_weight,'$table_weight');
  	$form['table'] = array('#theme' => 'table', '#header' => $header, '#rows' => $rows, '#weight' => $table_weight, '#empty' => "Empty table.", );
  	$output .= drupal_render($form['table']);
  }
  else {
  	//debug('theme_fetchit7_egrid_form 3');
    $row_count = $form['row_count']['#value'];
    $field_count = $form['field_count']['#value'];
    for ($row_ndx = 0; $row_ndx < $row_count; $row_ndx++) {
	    for ($fld_ndx = 0; $fld_ndx < $field_count; $fld_ndx++) {
        $element_name = fetchit_make_element_name($nid, $row_ndx, $fld_ndx);
				$hidden = fetchit_booleantointeger($form['param_hidden'][$fld_ndx]['#value']);
	    	if (!$hidden) { // hidden fields should have been created with a non-renderable control element type
        	$output .= drupal_render($form['elements'][$element_name]);
				}
   		}
  	}
  }
	//debug('theme_fetchit7_egrid_form 4');
	
  $output .= '</div>';
  $submit_button_flag = $form['submit_button_flag']['#value'];
  if ($submit_button_flag) {
    $output .= '<div class="' . $form_id . '_submit">';
    $output .= drupal_render($form['actions']['submit']);
    $output .= '</div>';
  }
  $addbody_flag = $form['addbody_flag']['#value'];
  if ($addbody_flag) {
    $output .= '<div class="' . $form_id . '_body">';
    $output .= drupal_render($form['body']);
    $output .= '</div>';
  }
  $output .= '<div class="' . $form_id . '_other">';
  $output .= drupal_render_children($form);
  $output .= '</div>';

  return $output;
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

function fetchit7_node_field_mapper($field_id) {
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
function fetchit7_content_field_mapper($field_id) {
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
    // fetchit7_field_par_tbl_rows
    case 'param_table_rows':
    case 'parameter_table_rows':
    case 'par_tbl_rows':
    case 'fetchit7_field_par_tbl_rows':
      $field_name = 'fetchit7_field_par_tbl_rows';
      break;
    // fetchit7_field_table_headers
    case 'table_headers':
    case 'fetchit7_field_table_headers':
      $field_name = 'fetchit7_field_table_headers';
      break;
		case 'table_empty_message':
		case 'table_emptymsg':
		case 'fetchit7_field_table_emptymsg':
      $field_name = 'fetchit7_field_table_emptymsg';
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
    case 'par_fld_weight':
    case 'fetchit7_field_par_fld_weight':
      $field_name = 'fetchit7_field_par_fld_weight';
      break;
    case 'par_fld_enabled':
    case 'fetchit7_field_par_fld_enabled':
      $field_name = 'fetchit7_field_par_fld_enabled';
      break;
    case 'par_fld_required':
    case 'fetchit7_field_par_fld_required':
      $field_name = 'fetchit7_field_par_fld_required';
      break;
    case 'par_fld_hidden':
    case 'fetchit7_field_par_fld_hidden':
      $field_name = 'fetchit7_field_par_fld_hidden';
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
    case 'egrid_append':
    case 'fetchit7_field_egrid_append':
      $field_name = 'fetchit7_field_egrid_append';
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
function fetchit7_content_field_fetcher($nid, $field_id = 'title', $format = 'value') {
  //$output = array();
  if ($node = node_load($nid)) {
    if ($field_name = fetchit7_node_field_mapper($field_id)) {
      // it is a recognized node field so get the value from $node object
      //TODO: fetchit7_content_field_fetcher only outputs value for node fields, add the other output types (value will work for now, so this is a low priority)
      $output = $node -> $field_name;
    }
    elseif ($field_name = fetchit7_content_field_mapper($field_id)) {
      // assume it is a custom content field and use field_get_items() to get the value
      $entity_type = 'node';
      $items = field_get_items($entity_type, $node, $field_name);
      //debug($items,$field_name.' $items');
      switch ($format) {
        case 'item':
          $output = $items[0];
          break;
        case 'items':
          $output = $items;
          break;
        case 'display':
          $output = field_view_value($entity_type, $node, $field_name, $items[0]);
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
    }
    else {
      // for now do nothing
      //debug($nid,'nid');
      //debug($field_id,'field_id');
      //debug($format,'format');
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
  if ($db_type = fetchit7_content_field_fetcher($nid, 'db_type', 'value')) {
    switch ($db_type) {
      case FETCHIT7_DBTYPE_NAME_PGSQL:
        //'postgresql':
        // these are required for remote pg db connections
        if (!strlen(trim($db_host = fetchit7_content_field_fetcher($nid, 'db_host', 'value')))) {
          return $db_handle;
        }
        if (!strlen(trim($db_port = fetchit7_content_field_fetcher($nid, 'db_port', 'value')))) {
          return $db_handle;
        }
        if (!strlen(trim($db_name = fetchit7_content_field_fetcher($nid, 'db_name', 'value')))) {
          return $db_handle;
        }
        if (!strlen(trim($db_login = fetchit7_content_field_fetcher($nid, 'db_login', 'value')))) {
          return $db_handle;
        }
        if (!strlen(trim($db_password = fetchit7_content_field_fetcher($nid, 'db_password', 'value')))) {
          return $db_handle;
        }
        // create the pg connection
        $db_string = "host=$db_host port=$db_port user=$db_login password=$db_password dbname=$db_name";
		//debug($db_string,'fetchit_get_db_handle '.$nid.' $db_string');
        $db_handle = pg_connect($db_string);
		//debug($db_handle,'fetchit_get_db_handle '.$nid.' $db_handle');
        break;
      case FETCHIT7_DBTYPE_NAME_MYSQL:
        //'mysql':
        break;
      case FETCHIT7_DBTYPE_NAME_MSSQL:
        //'sqlserver':
        break;
      default:
    }
  }
  return $db_handle;
}

function fetchit_run_query($db_type, $db_handle, $query) {
	//debug($db_type,'fetchit_run_query $db_type');
	//debug($db_handle,'fetchit_run_query $db_handle');
	//debug($query,'fetchit_run_query $query');
	switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      $result = pg_query($db_handle, $query);
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      break;
    default:
  }
  return $result;
}

function fetchit_get_field_count($db_type, &$data_object) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      if (!$field_count = pg_num_fields($data_object)) {
        return FALSE;
      }
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $field_count;
}

function fetchit_get_row_count($db_type, &$data_object) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      if (!$row_count = pg_num_rows($data_object)) {
        return FALSE;
      }
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $row_count;
}

function fetchit_get_field_names($db_type, &$data_object) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      if ($field_count = pg_num_fields($data_object)) {
        $field_names = array();
        for ($j = 0; $j < $field_count; $j++) {
          $field_names[] = pg_field_name($data_object, $j);
        }
      }
      else {
        return FALSE;
      }
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $field_names;
}

function fetchit_get_field_types($db_type, &$data_object) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      if ($field_count = pg_num_fields($data_object)) {
        $field_types = array();
        for ($j = 0; $j < $field_count; $j++) {
          $field_types[] = pg_field_type($data_object, $j);
        }
      }
      else {
        return FALSE;
      }
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $field_types;
}

function fetchit_get_data_object($db_type, $db_handle, $view_name, $user_uid_field, $limit = 0) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      $query = "SELECT * FROM $view_name";
      if (strlen($user_uid_field)) {
        global $user;
        $user_uid = $user -> uid;
        $query .= " WHERE $user_uid_field = " . $user_uid;
      }
      if ($limit) {
        $query .= " LIMIT $limit";
      }
      $data_object = pg_query($db_handle, $query);
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $data_object;
}

function fetchit_get_data_object_1($db_type, $db_handle, $view_name, $user_uid_field) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      $query = "SELECT * FROM $view_name";
      if (strlen($user_uid_field)) {
        global $user;
        $user_uid = $user -> uid;
        $query .= " WHERE $user_uid_field = " . $user_uid;
      }
      $query .= " LIMIT 1";
      $data_object = pg_query($db_handle, $query);
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $data_object;
}

function fetchit_get_data($db_type, &$data_object, $row, $field) {
  switch ($db_type) {
    case FETCHIT7_DBTYPE_NAME_PGSQL:
      //'postgresql':
      $data = pg_fetch_result($data_object, $row, $field);
      break;
    case FETCHIT7_DBTYPE_NAME_MYSQL:
      //'mysql':
      return FALSE;
      break;
    case FETCHIT7_DBTYPE_NAME_MSSQL:
      //'sqlserver':
      return FALSE;
      break;
    default:
      return FALSE;
  }
  return $data;
}

function fetchit_remove_records($db_handle, $db_type, $deletion_table, $user_uid_field) {
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($deletion_table))) {
            $qry = "DELETE FROM $deletion_table";
            if (strlen(trim($user_uid_field))) {
              global $user;
              $user_uid = $user -> uid;
              $qry .= " WHERE $user_uid_field = $user_uid";
            }
            $db_result = pg_query($db_handle, $qry);
            return TRUE;
          }
          else {
            return FALSE;
          }
          break;
        case FETCHIT7_DBTYPE_NAME_MYSQL:
        case FETCHIT7_DBTYPE_NAME_MSSQL:
        default:
          return FALSE;
      }
    }
    else {
      return FALSE;
    }
  }
  else {
    return FALSE;
  }
}

function fetchit_get_existing_selection_nid($db_handle, $db_type, $nid) {
  if ($db_handle) {
    if ($db_type) {
      if ($nid) {
        $table_selection_type = trim(fetchit7_content_field_fetcher($nid, 'table_selection_type', 'value'));
        switch ($table_selection_type) {
          case 'none':
            // static html - no selection - do nothing
            break;

          case 'copy_single':
            // radio button record selection
            $table_selection_table = trim(fetchit7_content_field_fetcher($nid, 'table_selection_table', 'value'));
            $table_selection_table = fetchit7_strip_special_char_default($table_selection_table);
            $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
            $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
            $result = fetchit_get_existing_selection_single($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field);
            return $result;
            break;

          case 'copy_multiple':
            // check boxes button record selection
            $table_selection_table = trim(fetchit7_content_field_fetcher($nid, 'table_selection_table', 'value'));
            $table_selection_table = fetchit7_strip_special_char_default($table_selection_table);
            $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
            $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
            $result = fetchit_get_existing_selection_multiple($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field);
            return $result;
            break;

          case 'delete_multiple':
          default:
        }
      }
    }
  }
}

function fetchit_set_existing_selection_nid($db_handle, $db_type, $nid, $val) {
  if ($db_handle) {
    if ($db_type) {
      if ($nid) {
        $table_selection_type = trim(fetchit7_content_field_fetcher($nid, 'table_selection_type', 'value'));
        switch ($table_selection_type) {
          case 'none':
            // static html - no selection - do nothing
            break;

          case 'copy_single':
            // radio button record selection
            $table_selection_table = trim(fetchit7_content_field_fetcher($nid, 'table_selection_table', 'value'));
            $table_selection_table = fetchit7_strip_special_char_default($table_selection_table);
            $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
            $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
            $result = fetchit_set_existing_selection_single($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field, $val);
            return $result;
            break;

          case 'copy_multiple':
            // check boxes button record selection
            $table_selection_table = trim(fetchit7_content_field_fetcher($nid, 'table_selection_table', 'value'));
            $table_selection_table = fetchit7_strip_special_char_default($table_selection_table);
            $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
            $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
            $result = fetchit_set_existing_selection_multiple($db_handle, $db_type, $table_selection_table, $key_field, $user_uid_field, $val);
            return $result;
            break;

          case 'delete_multiple':
            $deletion_table_name = trim(fetchit7_content_field_fetcher($nid, 'source_view', 'value'));
            $user_uid_field = trim(fetchit7_content_field_fetcher($nid, 'uid_field', 'value'));
            $key_field = trim(fetchit7_content_field_fetcher($nid, 'key_field', 'value'));
            $result = fetchit_del_existing_selection_multiple($db_handle, $db_type, $deletion_table_name, $key_field, $user_uid_field, $val);
            return $result;
            break;

          default:
        }
      }
    }
  }
}

function fetchit_get_existing_selection_single($db_handle, $db_type, $record_select_table, $key_field, $user_uid_field) {
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($record_select_table))) {
            if (strlen(trim($key_field))) {
              $qry = "SELECT $key_field FROM $record_select_table";
              if (strlen(trim($user_uid_field))) {
                global $user;
                $user_uid = $user -> uid;
                $qry .= " WHERE $user_uid_field = $user_uid";
              }
              $qry .= " LIMIT 1";
              $db_object = pg_query($db_handle, $qry);
              $row = pg_fetch_object($db_object);
              if (is_object($row)) {
                $selection = $row -> $key_field;
              }
              else {
                $selection = NULL;
              }
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

function fetchit_set_existing_selection_single($db_handle, $db_type, $record_select_table, $key_field, $user_uid_field, $val) {
  $insertedrecordcount = 0;
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($record_select_table))) {
            if (strlen(trim($key_field))) {
              $qry = "DELETE FROM $record_select_table";
              if (strlen(trim($user_uid_field))) {
                global $user;
                $qry .= " WHERE $user_uid_field = " . $user -> uid;
              }
              $db_result = pg_query($db_handle, $qry);
              $fldstr = $key_field;
              if (strlen(trim($user_uid_field))) {
                $fldstr .= ",$user_uid_field";
              }

              $qry = "INSERT INTO $record_select_table ($fldstr) VALUES (" . pg_escape_string($val);
              if (strlen(trim($user_uid_field))) {
                global $user;
                $user_uid = $user -> uid;
                $qry .= ",$user_uid";
              }
              $qry .= ")";
              if ($db_result = pg_query($db_handle, $qry)) {
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

function fetchit_get_existing_selection_multiple($db_handle, $db_type, $record_select_table, $key_field, $user_uid_field) {
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($record_select_table))) {
            if (strlen(trim($key_field))) {
              $qry = "SELECT $key_field FROM $record_select_table";
              if (strlen(trim($user_uid_field))) {
                global $user;
                $user_uid = $user -> uid;
                $qry .= " WHERE $user_uid_field = $user_uid";
              }
              $selection = array();
              $db_object = pg_query($db_handle, $qry);
              while ($row = pg_fetch_object($db_object)) {
                $selection[] = $row -> $key_field;
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

function fetchit_set_existing_selection_multiple($db_handle, $db_type, $record_select_table, $key_field, $user_uid_field, $vals) {
  $insertedrecordcount = 0;
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($record_select_table))) {
            if (strlen(trim($key_field))) {
              $qry = "DELETE FROM $record_select_table";
              if (strlen(trim($user_uid_field))) {
                global $user;
                $qry .= " WHERE $user_uid_field = " . $user -> uid;
              }
              $db_object = pg_query($db_handle, $qry);
              $fldstr = $key_field;
              if (strlen(trim($user_uid_field))) {
                $fldstr .= ",$user_uid_field";
              }
              foreach ($vals as $val) {
                $qry = "INSERT INTO $record_select_table ($fldstr) VALUES (" . pg_escape_string($val);
                if (strlen(trim($user_uid_field))) {
                  global $user;
                  $user_uid = $user -> uid;
                  $qry .= ",$user_uid";
                }
                $qry .= ")";
                if ($db_result = pg_query($db_handle, $qry)) {
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

function fetchit_del_existing_selection_multiple($db_handle, $db_type, $deletion_table, $key_field, $user_uid_field, $vals) {
  $deletedrecordcount = 0;
  if ($db_handle) {
    if ($db_type) {
      switch ($db_type) {
        case FETCHIT7_DBTYPE_NAME_PGSQL:
          if (strlen(trim($deletion_table))) {
            if (strlen(trim($key_field))) {
              foreach ($vals as $val) {
                $qry = "DELETE FROM $deletion_table WHERE $key_field = $val";
                if (strlen(trim($user_uid_field))) {
                  global $user;
                  $user_uid = $user -> uid;
                  $qry .= " AND $user_uid_field = $user_uid";
                }
                if ($db_result = pg_query($db_handle, $qry)) {
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

function fetchit_make_element_name($node_id, $row, $column) {
  //return 'fetchit_element_'.$node_id.'_'.$row.'_'.$column;
  return 'fetchit7_element_' . $node_id . '_' . $row . '_' . $column;
}

function fetchit_parse_parameter_string($parameter_name, $parameter_string) {
  $list_delimiter = ";";
  $pair_delimiter = ":";
  // make the list
  if (strlen(trim($parameter_name))) {
    if (strlen(trim($parameter_string))) {
      if (strpos($parameter_string, $pair_delimiter)) {
        $paramlist = array();
        $paramlist = explode($list_delimiter, $parameter_string);
        $param = array();
        foreach ($paramlist as $paramstr) {
          $tmp = array();
          $tmp = explode($pair_delimiter, $paramstr);
          $param[$tmp[0]] = $tmp[1];
        }
        if (array_key_exists($parameter_name, $param)) {
          return $param[$parameter_name];
        }
      }
    }
  }
  // search the list
  return null;
}

//function fetchit7_param_element_form ($form,&$form_state,$field_name,$field_type,$db_handle,$user_field_ndx,$user_field_data,$nid,$showtitles,$db_value) {
function fetchit7_param_element_form($field_name, $field_type, $db_handle, $user_field_ndx, $user_field_data, $nid, $showtitles, $db_value, $enabled=TRUE, $hidden=FALSE) {
	//debug('fetchit7_param_element_form passed in values:'.$field_name.', '.$field_type.', '.$user_field_ndx.', '.$db_value,'$field_name, $field_type, $user_field_ndx, $db_value');
	//debug($user_field_data,'$user_field_data');
  $form = array();
  switch (strtolower(trim($field_type))) {
    case 'bool':
      // a boolean from a pg database
      if ($hidden) return array('#type' => 'value','#value' => fetchit_booleantointeger($db_value));
      if ($user_field_ndx) {// if the user specified something this is set to the control index+1, else it is 0 to use the default for the field type
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DROPDOWN_SINGLE:
            return fetchit_get_select_element_bool($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_RADIOBTN:
            return fetchit_get_radios_element_bool($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 7;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOX:
            return fetchit_get_checkbox_element_bool($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOXES:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          case F_DR_CONTROL_DATESEL:
          case F_DR_CONTROL_DATETIMESEL:
          case F_DR_CONTROL_POPUPDATE:
            break;
          default:
            return fetchit_get_checkbox_element_bool($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
        }
      }
      else { // create the default input control element for booleans - a single check box
        if (strlen(trim($db_value))) {
          $default = filter_var(trim($db_value), FILTER_VALIDATE_BOOLEAN);
        } else {
        	$default = 0;
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array('#type' => 'checkbox', '#title' => $title, '#description' => $description, '#default_value' => $default, '#return_value' => 1, '#disabled' => !$enabled, '#required' => FALSE, '#weight' => $weight, );
        }
        else {
          $title = $field_name;
          $form = array('#type' => 'checkbox', '#title' => $title, '#default_value' => $default, '#return_value' => 1, '#disabled' => !$enabled, '#required' => FALSE, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    case 'int2':
    // a small integer from a pg database
    case 'int4':
    // an integer from a pg database
    case 'int8':
      // a long integer from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {// the user specified something
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DROPDOWN_SINGLE:
            return fetchit_get_select_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_RADIOBTN:
            return fetchit_get_radios_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 7;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOXES:
            return fetchit_get_checkboxes_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_MULTI:
            return fetchit_get_selectmultiple_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_DATETIMESEL:
          case F_DR_CONTROL_POPUPDATE:
          case F_DR_CONTROL_DATESEL:
          default:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 7;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
        }
      }
      else {// create the default input control element for integers - a text box
        //$parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
        $parameter_string = '';
        $size_string = fetchit_parse_parameter_string('size', $parameter_string);
        if (strlen(trim($size_string))) {
          $size = (int)$size_string;
        }
        else {
          $size = 7;
        }
        if (strlen(trim($db_value))) {
          $default = (int)$db_value;
        } else {
        	$default = '';
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        else {
          $form = array('#type' => 'textfield', '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    case 'numeric':
    // a "numeric" real number field from a pg database
    case 'float4':
    // a single precision real number field from a pg database
    case 'float8':
      // a double precision real number field from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {// the user specified something
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DROPDOWN_SINGLE:
            return fetchit_get_select_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_RADIOBTN:
            return fetchit_get_radios_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 9;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOXES:
            return fetchit_get_checkboxes_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_MULTI:
            return fetchit_get_selectmultiple_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_DATETIMESEL:
          case F_DR_CONTROL_POPUPDATE:
          case F_DR_CONTROL_DATESEL:
          default:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 9;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
        }
      }
      else {// create the default input control element for reals - a text box
        $default = $db_value;
        //$parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
        $parameter_string = '';
        $size_string = fetchit_parse_parameter_string('size', $parameter_string);
        if (strlen(trim($size_string))) {
          $size = (int)$size_string;
        }
        else {
          $size = strlen($default)+4;;
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        else {
          $form = array('#type' => 'textfield', '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    case 'date':
      // a date field from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {// the user specified something
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DATESEL:
            return fetchit_get_date_element($user_field_data, $user_field_ndx - 1, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DATETIMESEL:
            $date_format = 'M j, Y';
            $year_range = '-15:+15';
            return fetchit_get_date_select_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_POPUPDATE:
            $date_format = 'M j, Y';
            $year_range = '-15:+15';
            return fetchit_get_date_popup_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = strlen(date('M j, Y'))+3;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_SINGLE:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          case F_DR_CONTROL_RADIOBTN:
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_CHECKBOXES:
          default:
            return fetchit_get_date_element($user_field_data, $user_field_ndx - 1, $showtitles, $db_value);
        }
      }
      else {// the default input control element for dates - the drupal 6 core date element
    		$default = array(
					'year' => (int) date('Y'),
					'month' => (int) date('n'),
					'day' => (int) date('j'),
				);
        if (isset($db_value)) {
        	$default = array(
        		'year' => (int) date('Y', strtotime($db_value)),
        		'month' => (int) date('n', strtotime($db_value)),
        		'day' => (int) date('j', strtotime($db_value)),
					);
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array(
          	'#type' => 'date',
          	'#title' => $title,
          	'#description' => $description,
          	'#required' => FALSE,
          	'#disabled' => !$enabled,
          	'#default_value' => $default,
          	'#weight' => $weight,
					);
        }
        else {
          $form = array(
          	'#type' => 'date',
          	'#required' => FALSE,
          	'#disabled' => !$enabled,
          	'#default_value' => $default,
          	'#weight' => $weight,
					);
        }
        return $form;
      }
      break;
    case 'time':
    // a time field from a pg database
    case 'timetz':
      // a time field with timezone from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DATETIMESEL:
            $date_format = 'g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_select_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_POPUPDATE:
            $date_format = 'g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_popup_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
          case F_DR_CONTROL_DATESEL:
          case F_DR_CONTROL_TEXTBOX:
            $date_format = 'g:i A';
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
            	$size = strlen(date($date_format))+3;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_SINGLE:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          case F_DR_CONTROL_RADIOBTN:
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_CHECKBOXES:
          default:
            $date_format = 'g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_select_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
        }
      }
      else {// the default input control element for times - the drupal 6 date module api element, date_select formatted to only show hours and minutes
        $date_format = 'g:i A';
      	$default = date($date_format);
        if (strlen(trim($db_value))) {
          $default = date($date_format,strtotime($db_value));
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
				$size = strlen($default)+3;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          //$form = array('#type' => 'date_select', '#title' => $title, '#description' => $description, '#date_format' => $date_format, '#default_value' => $default, '#date_label_position' => 'none', //'none' 'within' 'above'
          //'#weight' => $weight, );
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        } else {
          //$form = array('#type' => 'date_select', '#date_format' => $date_format, '#default_value' => $default, '#date_label_position' => 'none', //'none' 'within' 'above'
          //'#weight' => $weight, );
          $form = array('#type' => 'textfield', '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    case 'timestamp':
    // a timestamp field from a pg database
    case 'timestamptz':
      // a timestamp field with timezone from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DATETIMESEL:
            $date_format = 'M j, Y g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_select_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_POPUPDATE:
            $date_format = 'M j, Y g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_popup_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
	            $date_format = 'M j, Y g:i A';
            	$size = strlen(date($date_format))+3;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DATESEL:
          case F_DR_CONTROL_DROPDOWN_SINGLE:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          case F_DR_CONTROL_RADIOBTN:
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_CHECKBOXES:
          default:
            $date_format = 'M j, Y g:i A';
            $year_range = '-15:+15';
            return fetchit_get_date_select_element($user_field_data, $user_field_ndx - 1, $date_format, $year_range, $showtitles, $db_value);
        }
      }
      else {// the default input control element for timestamps - the drupal 6 date module api element, date_select formatted to show months, days, years, hours and minutes
        $date_format = 'M j, Y g:i A';
    		$default = date($date_format);
        $year_range = '-15:+15';
        if (strlen(trim($db_value))) {
          $default = date($date_format,strtotime($db_value));
        }
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
				$size = strlen($default)+3;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          //$form = array('#type' => 'date_select', '#title' => $title, '#description' => $description, '#date_format' => $date_format, '#date_label_position' => 'none', //'none' 'within' 'above'
          //'#date_year_range' => $year_range, '#default_value' => $default, '#weight' => $weight, );
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        } else {
          //$form = array('#type' => 'date_select', '#date_format' => $date_format, '#date_label_position' => 'none', //'none' 'within' 'above'
          //'#date_year_range' => $year_range, '#default_value' => $default, '#weight' => $weight, );
          $form = array('#type' => 'textfield', '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    case 'text':
    // a text field from a pg database
    case 'varchar':
    // a varchar field from a pg database
    case 'bpchar':
      // a bpchar field from a pg database
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {// the user specified something
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_DROPDOWN_SINGLE:
            return fetchit_get_select_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_RADIOBTN:
            return fetchit_get_radios_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 20;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOXES:
            return fetchit_get_checkboxes_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_MULTI:
            return fetchit_get_selectmultiple_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_DATESEL:
          case F_DR_CONTROL_DATETIMESEL:
          case F_DR_CONTROL_POPUPDATE:
          default:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 20;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
        }
      }
      else {// the default input control element for text fields - a text box
        $parameter_string = '';
        $size_string = fetchit_parse_parameter_string('size', $parameter_string);
        if (strlen(trim($size_string))) {
          $size = (int)$size_string;
        }
        else {
          $size = 20;
        }
        $default = $db_value;
        //$weight = $user_field_ndx - 1;
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        } else {
          $form = array('#type' => 'textfield', '#default_value' => $default, '#size' => $size, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
      break;
    default:
      // an unanticipated field type, put a text box or single selects, but may not work
      if ($hidden) return array('#type' => 'value','#value' => $db_value);
      if ($user_field_ndx) {// the user specified something
        switch ($user_field_data[$user_field_ndx-1][F_DR_FIELD_INPUT_TYPE_STR]) {
          case F_DR_CONTROL_TEXTBOX:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 20;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_DROPDOWN_SINGLE:
            return fetchit_get_select_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_RADIOBTN:
            return fetchit_get_radios_element($user_field_data, $user_field_ndx - 1, $db_handle, $nid, $showtitles, $db_value);
            break;
          case F_DR_CONTROL_CHECKBOXES:
          case F_DR_CONTROL_CHECKBOX:
          case F_DR_CONTROL_DROPDOWN_MULTI:
          case F_DR_CONTROL_DATESEL:
          default:
            $parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
            $size_string = fetchit_parse_parameter_string('size', $parameter_string);
            if (strlen(trim($size_string))) {
              $size = (int)$size_string;
            }
            else {
              $size = 20;
            }
            return fetchit_get_textbox_element($user_field_data, $user_field_ndx - 1, $size, $showtitles, $db_value);
        }
      }
      else {// try a text box for this unanticipated field type
        //$parameter_string = $user_field_data[$user_field_ndx - 1][F_DR_FIELD_PARAMETER_STR];
				$parameter_string = '';
        $size_string = fetchit_parse_parameter_string('size', $parameter_string);
        $size_string = '';
        if (strlen(trim($size_string))) {
          $size = (int)$size_string;
        }
        else {
          $size = 40;
        }
        $default = $db_value;
        //if (strlen(trim($user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR]))) {
        //  $weight = (int)$user_field_data[$user_field_ndx - 1][FETCHIT7_FIELD_WEIGHT_STR];
        //}
        //else {
        //  $weight = $user_field_ndx - 1;
        //}
        $weight = 0;
        if ($showtitles) {
          $title = $field_name;
          $description = $field_type;
          $form = array('#type' => 'textfield', '#title' => $title, '#description' => $description, '#size' => $size, '#default_value' => $default, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        else {
          $form = array('#type' => 'textfield', '#size' => $size, '#default_value' => $default, '#required' => FALSE, '#disabled' => !$enabled, '#weight' => $weight, );
        }
        return $form;
      }
  }
  return $form;
}

function fetchit_get_textbox_element($user_field_data, $user_field_ndx, $size, $showtitles, $db_value) {
  // $db_value is a scalar, NOT an array

  $default = '';
  $maxlength = 255;

  // process the value stored in the output table already for this parameter/user
  // convert whatever it is to a string...this is a text box, after all!
  if (isset($db_value)) {
    $default = print_r($db_value, TRUE);
  }
	//debug($default,'$default');
  // now overwrite it with the user entered value - even if blank - which it now might be if the param function did its job
  // unless it has a special special value
  if (strlen(trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
	  switch ($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	      // leave it set to the at the db value
	      break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	      $default = '';
	      break;
	    default: // it is not blank, it is not, one of the known strings, so use it
	      $default = $user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR];
	  }
  } else {
  	// do nothing, use the existing default
  }

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  //debug($default,'textbox $default');
  //debug($required,'textbox $required');
  //debug($disabled,'textbox $disabled');
  //debug($weight,'textbox $weight');
  if ($showtitles) {
    $title = $user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR];
    $description = $user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR];
    $form = array('#title' => $title, '#type' => 'textfield', '#size' => $size, '#maxlength' => $maxlength, '#default_value' => $default, '#required' => $required, '#disabled' => $disabled, '#description' => $description, '#weight' => $weight, );
  }
  else {
    $form = array('#type' => 'textfield', '#size' => $size, '#maxlength' => $maxlength, '#default_value' => $default, '#required' => $required, '#disabled' => $disabled,
    //'#weight' => $weight,
    );
  }

  return $form;
}

function fetchit_get_checkboxes_element($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_values) {
  //note that $db_values should be an array

  $form = array();

  // create the $options array containing the selectable items
  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a string, and therefore the output table has to be able to handle it, though make_val() will try to cast it as best it can
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    $options = array();
    foreach ($vals as $val) {
      $options[$val] = $val;
    }
  }
  else {
    // create it from a table in the database
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options = array();
      if (!$options = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid)) {
        return $form;
      }
    }
    else {
      return $form;
    }
  }

  $options_keys = array_keys($options);

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $defaults = array($options_keys[0]);
  }
  else {
    $defaults = array();
  }
	
	// if it is set, use the passed in default
	if (isset($db_values)) {
		if (is_array($db_values)) {
			if (count($db_values)) {
				$found_values = array();
				foreach ($db_values as $db_value) {
					if (in_array($db_value, $options_keys)) {
						$found_values[] = $db_value;
					}
				}
				$defaults = $found_values;
			} else {
				$defaults = array();
			}
		} else {
			$defaults = array();
		}
	} else {
		$defaults = array();
	}
	
  // then set up the defaults based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the values they entered last time
	      // already entered
	      break;
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
	      $defaults = array($options_keys[0]);
	      break;
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
	      $defaults = array($options_keys[1]);
	      break;
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	      $defaults = array_keys($options);
	      break;
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	      $defaults = array();
	      break;
	    default:
	      $db_values = explode(',', $entered);
				$found_values = array();
				foreach ($db_values as $db_value) {
					if (in_array($db_value, $options_keys)) {
						$found_values[] = $db_value;
		    	}
				}
				$defaults = $found_values;
	  }
	}

  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'checkboxes',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $defaults,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
		);
  }
  else {
    $form = array(
    	'#type' => 'checkboxes',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $defaults,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
		);
  }

  return $form;
}

function fetchit_get_checkbox_element_bool($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_value) {
  //note that $db_value should be a scalar, not an array
	//debug('passed into checkbox routine: $db_value='.$db_value);
  $form = array();

  // create the $options array containing the selectable items
  //here is a beginning options array
  $options = array();
  $options[0] = "false";
  $options[1] = "true";
  // this is the string that would be displayed next to the checkbox, except that we using the option to put the control title there

  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a 0,1 integer value
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    switch (count($vals)) {
      case 0:
        $options[0] = 'false';
        $options[1] = 'true';
        break;
      case 1:
        $options[0] = $vals[0];
        $options[1] = $vals[0];
        break;
      case 2:
      default:
        $options[0] = $vals[0];
        $options[1] = $vals[1];
    }
  }
  else {
    // create it from a table in the database
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options_tmp = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid);
      switch (count($options_tmp)) {
        case 0:
          $options[0] = 'false';
          $options[1] = 'true';
          break;
        case 1:
          $options[0] = $options_tmp[0];
          $options[1] = $options_tmp[0];
          break;
        case 2:
        default:
          $options[0] = $options_tmp[0];
          $options[1] = $options_tmp[1];
      }
    }
  }

	$options_keys = array_keys($options);
	
  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];
  // avoid setting this to true because it essentially disables the checkbox
  $required = FALSE;

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // set up the beginning default based on the required flag
  if ($required) {
    $default = $options_keys[1];
  }
  else {
    $default = $options_keys[0];
  }

	// handle the default value passed in
	if (isset($db_value)) {
		$mykey = fetchit_booleantointeger($db_value);
    if (in_array($mykey, $options_keys)) {
      $default = $mykey;
    }
    else {
      $default = $options_keys[0];
    }
	}
	
  // then set up the default based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
	  switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the values they entered last time
	      // should already be set
	      break;
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	      $default = $options_keys[1];
	      break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
	      $default = $options_keys[0];
	      break;
	    default:
				$mykey = fetchit_booleantointeger($entered);
		    if (in_array($mykey, $options_keys)) {
		      $default = $mykey;
		    }
		    else {
		      $default = $options_keys[0];
		    }
	  }
  }
	//debug('converted inside checkbox routine: $default='.$default);

  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'checkbox',
    	'#required' => $required,
    	'#default_value' => $default,
    	'#return_value' => 1,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
		);
  }
  else {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'checkbox',
    	'#required' => $required,
    	'#default_value' => $default,
    	'#return_value' => 1,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
		);
  }

  return $form;
}

function fetchit_get_select_element($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_value) {
  //debug($db_value,'fetchit_get_select_element $db_value');
  // note that $db_value is a scalar, NOT an array

  $form = array();

  // create the $options array containing the selectable items
  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a string, and therefore the output table has to be able to handle it, though make_val() will try to cast it as best it can
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    $options = array();
    foreach ($vals as $val) {
      $options[$val] = $val;
    }
  }
  else {
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options = array();
      if (!$options = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid)) {
        return $form;
      }
    }
    else {
      return $form;
    }
  }

  $options_keys = array_keys($options);

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $default = $options_keys[0];
    $empty_value = '';
  }
  else {
    $default = $options_keys[0];
    $empty_value = '';
  }
	
	// if it is set, use the passed in default
	//debug('radiobuttons: $db_value = '.$db_value);
	if (isset($db_value)) {
    if (in_array($db_value, $options_keys)) {
      $default = $db_value;
    }
    else {
      $default = $options_keys[0];
    }
	}
	
  // then set up the defaults based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the value they entered last time
	      // it is already set
	      break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	      $default = '';
	      break;
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	      $default = $options_keys[0];
	      break;
	    default:
	      if (in_array($entered, $options_keys)) {
	        $default = $entered;
	      }
	      else {
	        $default = '';
	      }
	  }
	}
	
  //debug($default,'fetchit_get_select_element $default');
  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => FALSE,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }
  else {
    $form = array(
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => FALSE,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }

  return $form;
}

function fetchit_get_select_element_bool($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_value) {
  // create the $options array containing the selectable items
  //here is a beginning options array
  $options = array();
  $options[0] = "false";
  $options[1] = "true";

  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a 0,1 integer value
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    switch (count($vals)) {
      case 0:
        $options[0] = 0;
        $options[1] = 1;
        break;
      case 1:
        $options[0] = $vals[0];
        $options[1] = 1;
        break;
      case 2:
      default:
        $options[0] = $vals[0];
        $options[1] = $vals[1];
    }
  }
  else {
    // create it from a table in the database
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options_tmp = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid);
      switch (count($options_tmp)) {
        case 0:
          $options[0] = 0;
          $options[1] = 1;
          break;
        case 1:
          $options[0] = $options_tmp[0];
          $options[1] = 1;
          break;
        case 2:
        default:
          $options[0] = $options_tmp[0];
          $options[1] = $options_tmp[1];
      }
    }
  }

  $options_keys = array_keys($options);

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $default = $options_keys[0];
    //$default = '';
    $empty_value = '';
  }
  else {
    $default = $options_keys[0];
    //$default = '';
    $empty_value = '';
  }

	// handle the default value passed in
	//debug('radiobuttons: $db_value = '.$db_value);
	if (isset($db_value)) {
		$mykey = fetchit_booleantointeger($db_value);
    if (in_array($mykey, $options_keys)) {
      $default = $mykey;
    }
    else {
      $default = $options_keys[0];
    }
	}

  // then set up the defaults based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the value they entered last time
	      // it is already set
	      break;
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
		    $default = $options_keys[1];
				break;
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
		    $default = $options_keys[0];
				break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	      $default = '';
	      break;
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	      $default = $options_keys[0];
	      break;
	    default:
				$mykey = fetchit_booleantointeger($entered);
		    if (in_array($mykey, $options_keys)) {
		      $default = $mykey;
		    }
		    else {
		      $default = $options_keys[0];
		    }
  	}
	}
	
  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => FALSE,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }
  else {
    $form = array(
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => FALSE,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }

  return $form;
}

function fetchit_get_selectmultiple_element($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_values) {
  //note that $db_values should be an array

  $form = array();

  // create the $options array containing the selectable items
  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a string, and therefore the output table has to be able to handle it, though make_val() will try to cast it as best it can
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    $options = array();
    foreach ($vals as $val) {
      $options[$val] = $val;
    }
  }
  else {
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options = array();
      if (!$options = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid)) {
        return $form;
      }
    }
    else {
      return $form;
    }
  }

  $options_keys = array_keys($options);

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $defaults = array($options_keys[0]);
    $empty_value = '';
  }
  else {
    $defaults = array('');
    $empty_value = '';
  }

	// if it is set, use the passed in default
	if (isset($db_values)) {
		if (is_array($db_values)) {
			if (count($db_values)) {
				$found_values = array();
				foreach ($db_values as $db_value) {
					if (in_array($db_value, $options_keys)) {
						$found_values[] = $db_value;
					}
				}
				$defaults = $found_values;
			} else {
		    $defaults = array();
			}
		} else {
	    $defaults = array();
		}
	} else {
    	$defaults = array();
	}
	
	// then set up the defaults based on the string they typed into the defaults attribute box
	if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
		switch ($entered) {
			case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
				// over write defaults with the value they entered last time
				// already set
				break;
			case FETCHIT7_FIELD_DEFAULT_TRUE:
		    	$default = $options_keys[1];
				break;
			case FETCHIT7_FIELD_DEFAULT_FALSE:
				$default = $options_keys[0];
				break;
			case FETCHIT7_FIELD_DEFAULT_NONE:
	    	case FETCHIT7_FIELD_DEFAULT_BLANK:
	      		$defaults = array('');
	      		break;
	    	case FETCHIT7_FIELD_DEFAULT_FIRST:
	      		$defaults = array($options_keys[0]);
	      		break;
	    	case FETCHIT7_FIELD_DEFAULT_ALL:
				$defaults = $options_keys;
			break;
		default:
			$userdefaults = explode(',', $entered);
			if (count($userdefaults)) {
				$defaults = $userdefaults;
			} else {
	        	$defaults = array('');
	      	}
	  	}
	}

  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => TRUE,
    	'#required' => $required,
    	'#default_value' => $defaults,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }
  else {
    $form = array(
    	'#type' => 'select',
    	'#options' => $options,
    	'#multiple' => TRUE,
    	'#required' => $required,
    	'#default_value' => $defaults,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
    	'#empty_value' => $empty_value,
		);
  }

  return $form;
}

function fetchit_get_radios_element($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_value) {
  // note that $db_value is a scalar, NOT an array

  $form = array();

  // create the $options array containing the selectable items
  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a string, and therefore the output table has to be able to handle it, though make_val() will try to cast it as best it can
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    $options = array();
    foreach ($vals as $val) {
      $options[$val] = $val;
    }
  }
  else {
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options = array();
      if (!$options = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid)) {
        return $form;
      }
    }
    else {
      return $form;
    }
  }

  $options_keys = array_keys($options);

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $default = $options_keys[0];
  }
  else {
    $default = $options_keys[0];
  }
	
	// if it is set, use the passed in default
	//debug('radiobuttons: $db_value = '.$db_value);
	if (isset($db_value)) {
    if (in_array($db_value, $options_keys)) {
      $default = $db_value;
    }
    else {
      $default = $options_keys[0];
    }
	}
	
  // then set up the defaults based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
	  switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the value they entered last time - this is what is already in there
	      break;
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
		    $default = $options_keys[1];
				break;
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
		    $default = $options_keys[0];
				break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
				// just mark the first one
	      $default = $options_keys[0];
	      break;
	    default: // try to find the one they entered
	      if (in_array($entered, $options_keys)) {
	        $default = $entered;
	      }
	      else {
	        $default = $options_keys[0];
	      }
	  }
  }

  //debug($default,'fetchit_get_select_element $default');
  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'radios',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
		);
  }
  else {
    $form = array(
    	'#type' => 'radios',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
		);
  }

  return $form;
}

function fetchit_get_radios_element_bool($user_field_data, $user_field_ndx, $db_handle, $nid, $showtitles, $db_value) {
  // create the $options array containing the selectable items
  //here is a beginning options array
  $options = array();
  $options[0] = "false";
  $options[1] = "true";

  if (substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 0, 1) == F_DR_LIST_TBL_SPECIAL_CHAR) {
    // create it from a specially formatted input string in the input list table name
    // note in this case the key is a 0,1 integer value
    $vals = explode(",", substr($user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR], 1));
    switch (count($vals)) {
      case 0:
        $options[0] = 0;
        $options[1] = 1;
        break;
      case 1:
        $options[0] = $vals[0];
        $options[1] = 1;
        break;
      case 2:
      default:
        $options[0] = $vals[0];
        $options[1] = $vals[1];
    }
  }
  else {
    // create it from a table in the database
    $list_tbl = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_TBL_STR];
    if (strlen($list_tbl)) {
      $list_key_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_NDX_STR];
      $list_dsp_fld = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_LIST_DSP_STR];
      $list_uid_fld = $user_field_data[$user_field_ndx][F_DR_FIELD_LIST_UID_STR];
      $options_tmp = fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid);
      switch (count($options_tmp)) {
        case 0:
          $options[0] = 0;
          $options[1] = 1;
          break;
        case 1:
          $options[0] = $options_tmp[0];
          $options[1] = 1;
          break;
        case 2:
        default:
          $options[0] = $options_tmp[0];
          $options[1] = $options_tmp[1];
      }
    }
  }

	$options_keys = array_keys($options);
	
  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default values to be selected
  // start with a default selection based on whether the control requires a value or not
  if ($required) {
    $default = $options_keys[0];
  }
  else {
    $default = $options_keys[0];
  }

	// handle the default value passed in
	//debug('radiobuttons: $db_value = '.$db_value);
	if (isset($db_value)) {
		$mykey = fetchit_booleantointeger($db_value);
    if (in_array($mykey, $options_keys)) {
      $default = $mykey;
    }
    else {
      $default = $options_keys[0];
    }
	}

  // then set up the defaults based on the string they typed into the defaults attribute box
  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the value they entered last time
	      // it is already set
	      break;
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
		    $default = $options_keys[1];
				break;
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
		    $default = $options_keys[0];
				break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
		    $default = $options_keys[0];
				break;
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
		    $default = $options_keys[1];
				break;
	    default:
	      // since we know the keys are 0 and 1 in this specialized select control, go ahead and do the conversion
				$mykey = fetchit_booleantointeger($entered);
		    if (in_array($mykey, $options_keys)) {
		      $default = $mykey;
		    }
		    else {
		      $default = $options_keys[0];
		    }
  	}
	}

  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'radios',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#description' => $description,
    	'#weight' => $weight,
		);
  }
  else {
    $form = array(
    	'#type' => 'radios',
    	'#options' => $options,
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	'#weight' => $weight,
		);
  }

  return $form;
}

function fetchit_get_date_element($user_field_data, $user_field_ndx, $showtitles, $db_value) {
	//debug($db_value,'fetchit_get_date_element: passed in value $db_value');
  $form = array();

  // process the required attribute - should be boolean already
  $required = $user_field_data[$user_field_ndx][F_DR_FIELD_REQUIRED_STR];

  // process the disabled attribute - should be boolean already
  $disabled = $user_field_data[$user_field_ndx][F_DR_FIELD_DISABLED_STR];

  // the weight attribute - controls rendering order on the form - should be an integer already
  $weight = $user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];

  // figure out the default value
  // start with today's date if it is required or even if it is not :-)
  // use the system date as a default
  if ($required) {
    $default = array(
    	'month' => (int) date('n'),
    	'day' => (int) date('j'),
    	'year' => (int) date('Y'),
		);
  } else {
  	$default = array(
    	'month' => (int) date('n'),
    	'day' => (int) date('j'),
    	'year' => (int) date('Y'),
		);
  }

  // now replace it with the passed in default value if it is set...
  if (isset($db_value)) {
  	$mydate = strtotime($db_value);
    $default = array(
    	'month' => (int) date('n', strtotime($db_value)),
    	'day' => (int) date('j', strtotime($db_value)),
    	'year' => (int) date('Y', strtotime($db_value)),
		);
  }

  if (strlen($entered = trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
	  switch ($entered) {
	    case FETCHIT7_FIELD_DEFAULT_PREVIOUS:
	      // over write defaults with the value they entered last time
	      // these are already the default...
	      break;
	    case FETCHIT7_FIELD_DEFAULT_NONE:
	    case FETCHIT7_FIELD_DEFAULT_BLANK:
	    case FETCHIT7_FIELD_DEFAULT_FIRST:
	    case FETCHIT7_FIELD_DEFAULT_ALL:
	    case FETCHIT7_FIELD_DEFAULT_TRUE:
	    case FETCHIT7_FIELD_DEFAULT_FALSE:
				// can't do these, leave it at whatever it is
	      break;
	    default: // try to convert the entered value into a date
	      $default = array(
	      	'year' => (int) date('Y', strtotime($entered)),
	      	'month' => (int) date('n', strtotime($entered)),
	      	'day' => (int) date('j', strtotime($entered)),
				);
	  }
  }
	//debug($default,'fetchit_get_date_element: user entered default $default');

  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array(
    	'#title' => $title,
    	'#type' => 'date',
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	//'#process' => array('expand_date_fetchit'),
    	'#description' => $description, '#weight' => $weight,
		);
  } else {
    $form = array(
    	'#type' => 'date',
    	'#required' => $required,
    	'#default_value' => $default,
    	'#disabled' => $disabled,
    	//'#process' => array('expand_date_fetchit'),
    	'#weight' => $weight,
		);
  }

  return $form;
}

function fetchit_get_date_select_element($user_field_data, $user_field_ndx, $date_format, $year_range, $showtitles, $db_value) {
  $form = array();
  $default = format_date(time(), 'custom', 'Y-m-d H:i:s');
  if (strlen(trim($db_value))) {
    $default = format_date(strtotime($db_value), 'custom', 'Y-m-d H:i:s', 0);
  }
  if (strlen(trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    $default = format_date(strtotime($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]), 'custom', 'Y-m-d H:i:s', 0);
  }
  if (!strlen(trim($date_format)))
    $date_format = 'Y-m-d g:i:s A';
  if (!strlen(trim($year_range)))
    $year_range = '-15:+15';
  if (strlen(trim($user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR]))) {
    $weight = (int)$user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];
  }
  else {
    $weight = $user_field_ndx;
  }
  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array('#type' => 'date_select', '#title' => $title, '#default_value' => $default, '#date_label_position' => 'none', //'none' 'within' 'above'
    '#date_year_range' => $year_range, //'-3:+3'
    '#date_format' => $date_format, //'Y-m-d H:i' 'm-d-Y',
    '#description' => $description, '#weight' => $weight, );
  }
  else {
    $form = array('#type' => 'date_select', '#default_value' => $default, '#date_label_position' => 'none', //'none' 'within' 'above'
    '#date_year_range' => $year_range, //'-3:+3'
    '#date_format' => $date_format, //'Y-m-d H:i' 'm-d-Y',
    '#weight' => $weight, );
  }

  return $form;
}

function fetchit_get_date_popup_element($user_field_data, $user_field_ndx, $date_format, $year_range, $showtitles, $db_value) {
  $form = array();
  $default = format_date(time(), 'custom', 'Y-m-d H:i:s');
  if (strlen(trim($db_value))) {
    $default = format_date(strtotime($db_value), 'custom', 'Y-m-d H:i:s', 0);
  }
  if (strlen(trim($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]))) {
    $default = format_date(strtotime($user_field_data[$user_field_ndx][F_DR_FIELD_DEFAULT_STR]), 'custom', 'Y-m-d H:i:s', 0);
  }
  if (!strlen(trim($date_format)))
    $date_format = 'Y-m-d g:i:s A';
  if (!strlen(trim($year_range)))
    $year_range = '-15:+15';
  if (strlen(trim($user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR]))) {
    $weight = (int)$user_field_data[$user_field_ndx][FETCHIT7_FIELD_WEIGHT_STR];
  }
  else {
    $weight = $user_field_ndx;
  }
  if ($showtitles) {
    $title = t($user_field_data[$user_field_ndx][F_DR_FIELD_TITLE_STR]);
    $description = t($user_field_data[$user_field_ndx][F_DR_FIELD_DESC_STR]);
    $form = array('#title' => $title, '#type' => 'date_popup', '#default_value' => $default, '#date_format' => $date_format, '#date_year_range' => $year_range, //'-3:+3'
    '#description' => $description, '#weight' => $weight, );
  }
  else {
    $form = array('#type' => 'date_popup', '#default_value' => $default, '#date_format' => $date_format, '#date_year_range' => $year_range, //'-3:+3'
    '#weight' => $weight, );
  }

  return $form;
}

function fetchit_get_options_list($db_handle, $list_tbl, $list_uid, $nid) {
  $db_type = trim(fetchit7_content_field_fetcher($nid, 'db_type', 'value'));
  if (strlen($db_type)) {
    switch ($db_type) {
      case FETCHIT7_DBTYPE_NAME_PGSQL:
        $stored = F_DR_LIST_TBL_NDX_FLD;
        $displayed = F_DR_LIST_TBL_DSP_FLD;
        $db_query = "SELECT $stored,$displayed FROM $list_tbl";
        if (strlen(trim($list_uid))) {
          global $user;
          //$uid_field = F_DR_LIST_TBL_UID_FLD;
          $uid_field = $list_uid;
          $db_query .= " WHERE $uid_field = " . $user -> uid;
        }
        if ($db_object = pg_query($db_handle, $db_query)) {
          $options = array();
          while ($row = pg_fetch_object($db_object)) {
            $options[$row -> $stored] = $row -> $displayed;
          }
        }
        break;
      case FETCHIT7_DBTYPE_NAME_MYSQL:
      case FETCHIT7_DBTYPE_NAME_MSSQL:
      default:
    }
  }
  return $options;
}

function fetchit7_get_options_list($db_handle, $list_tbl, $list_key_fld, $list_dsp_fld, $list_uid_fld, $nid) {
  $options = array();
  $db_type = trim(fetchit7_content_field_fetcher($nid, 'db_type', 'value'));
  if (strlen($db_type)) {
    switch ($db_type) {
      case FETCHIT7_DBTYPE_NAME_PGSQL:
        $db_query = "SELECT $list_key_fld,$list_dsp_fld FROM $list_tbl";
        if (strlen($list_uid_fld)) {
          global $user;
          $db_query .= " WHERE $list_uid_fld = " . $user -> uid;
        }
        $options = array();
        if ($db_object = pg_query($db_handle, $db_query)) {
          while ($row = pg_fetch_object($db_object)) {
            $options[$row -> $list_key_fld] = $row -> $list_dsp_fld;
          }
        }
        break;
      case FETCHIT7_DBTYPE_NAME_MYSQL:
      case FETCHIT7_DBTYPE_NAME_MSSQL:
      default:
    }
  }
  return $options;
}

/**
 * Roll out a single date element.
 */
function expand_date_fetchit($element) {
  // Default to current date
  if (empty($element['#value'])) {
    $element['#value'] = array('day' => format_date(time(), 'custom', 'j'), 'month' => format_date(time(), 'custom', 'n'), 'year' => format_date(time(), 'custom', 'Y'));
  }

  $element['#tree'] = TRUE;

  // Determine the order of day, month, year in the site's chosen date format.
  $format = variable_get('date_format_short', 'm/d/Y - H:i');
  $sort = array();
  $sort['day'] = max(strpos($format, 'd'), strpos($format, 'j'));
  $sort['month'] = max(strpos($format, 'm'), strpos($format, 'M'));
  $sort['year'] = strpos($format, 'Y');
  asort($sort);
  $order = array_keys($sort);

  // Output multi-selector for date.
  foreach ($order as $type) {
    switch ($type) {
      case 'day':
        $options = drupal_map_assoc(range(1, 31));
        break;
      case 'month':
        $options = drupal_map_assoc(range(1, 12), 'map_month');
        break;
      case 'year':
        $options = drupal_map_assoc(range($minyear, $maxyear));
        break;
    }
    $parents = $element['#parents'];
    $parents[] = $type;
    $element[$type] = array('#type' => 'select', '#value' => $element['#value'][$type], '#attributes' => $element['#attributes'], '#options' => $options, '#weight' => $weight, );
  }
}

/*
 * creates the correct string out of the boolean variable value $val
 * to put inside VALUES() of a SQL INSERT statement
 * should be universal - only tested for pgSQL though
 *
 * $val is a boolean typed variable
 * $field_type is a string
 *
 */
function fetchit_make_val_str($val, $field_type) {
  //debug($val,'$val');
  //debug($field_type,'$field_type');
  switch (strtolower(trim($field_type))) {
    case 'bool':
    // a boolean from a pg database
    case 'boolean':
      // a generic type
      switch (strtolower(trim($val))) {
        case "":
          return "NULL";
        case "f":
        case "false":
        case "n":
        case "no":
        case "0":
          return "FALSE";
        case "t":
        case "true":
        case "y":
        case "yes":
        case "1":
        case "-1":
        default:
          return "TRUE";
      }
    case 'int2':
    // a small integer from a pg database
    case 'int4':
    // an integer from a pg database
    case 'int8':
    // a long integer from a pg database
    case 'integer':
      // a generic type
      if ($val == "") {
        return "NULL";
      }
      else {
        return strval(intval($val));
      }
    case 'numeric':
    // a "numeric" real number field from a pg database
    case 'float4':
    // a single precision real number field from a pg database
    case 'float8':
    // a double precision real number field from a pg database
    case 'real':
      // a generic type
      if ($val == "") {
        return "NULL";
      }
      else {
        return strval(floatval($val));
      }
    case 'text':
    // a text field from a pg database
    case 'varchar':
    // a varchar field from a pg database
    case 'bpchar':
      // a bpchar field from a pg database
      return "'" . pg_escape_string($val) . "'";
    case 'date':
      // a date field from a pg database
      if ($val == "") {
        return "NULL";
      }
      else {
        return "DATE '" . $val . "'";
      }
    case 'time':
    // a time field from a pg database
    case 'timetz':
      // a time field with timezone from a pg database
      if ($val == "") {
        return "NULL";
      }
      else {
        return "TIME '" . $val . "'";
      }
    case 'timestamp':
    // a timestamp field from a pg database
    case 'timestamptz':
      // a timestamp field with timezone from a pg database
      if ($val == "") {
        return "NULL";
      }
      else {
        return "TIMESTAMP '" . $val . "'";
      }
    default:
      // an unanticipated field type, put a text box or single selects, but may not work
      if ($val == "") {
        return "NULL";
      }
      else {
        return "'" . $val . "'";
      }
  }
}

/*
 * creates the correct string out of the boolean variable value $val
 * to put inside VALUES() of a SQL INSERT statement
 * when the target field is of type $field_type
 * default is that nulls coming in map to nulls going out
 * should be universal - only tested for pgSQL though
 *
 * $val is a boolean typed variable
 * $field_type is a string
 * returns a string with the necessary delimiters
 */
function fetchit7_boolean_to_sql_values_str($val, $field_type = 'boolean', $nulls = TRUE) {
  if (isset($val)) {// it likely will be by this point
    $boolval = (bool)$val;
    // just to be safe
    switch ($boolval) {
      case TRUE:
        switch (strtolower(trim($field_type))) {
          case 'bool':
          // a boolean from a pg database
          case 'boolean':
            // a generic type
            return "TRUE";
          case 'int2':
          // a small integer from a pg database
          case 'int4':
          // an integer from a pg database
          case 'int8':
          // a long integer from a pg database
          case 'integer':
            // a generic type
            return "1";
          case 'numeric':
          // a "numeric" real number field from a pg database
          case 'float4':
          // a single precision real number field from a pg database
          case 'float8':
          // a double precision real number field from a pg database
          case 'real':
            // a generic type
            return "1";
          case 'text':
          // a text field from a pg database
          case 'varchar':
          // a varchar field from a pg database
          case 'bpchar':
            // a bpchar field from a pg database
            return "'TRUE'";
          case 'date':
            // a date field from a pg database
            return "DATE '1'";
          case 'time':
          // a time field from a pg database
          case 'timetz':
            // a time field with timezone from a pg database
            return "TIME '1'";
          case 'timestamp':
          // a timestamp field from a pg database
          case 'timestamptz':
            // a timestamp field with timezone from a pg database
            return "TIMESTAMP '1'";
          default:
            // an unanticipated field type
            return "'1'";
        }
      case FALSE:
      default:
        switch (strtolower(trim($field_type))) {
          case 'bool':
          // a boolean from a pg database
          case 'boolean':
            // a generic type
            return "FALSE";
          case 'int2':
          // a small integer from a pg database
          case 'int4':
          // an integer from a pg database
          case 'int8':
          // a long integer from a pg database
          case 'integer':
            // a generic type
            return "0";
          case 'numeric':
          // a "numeric" real number field from a pg database
          case 'float4':
          // a single precision real number field from a pg database
          case 'float8':
          // a double precision real number field from a pg database
          case 'real':
            // a generic type
            return "0";
          case 'text':
          // a text field from a pg database
          case 'varchar':
          // a varchar field from a pg database
          case 'bpchar':
            // a bpchar field from a pg database
            return "'FALSE'";
          case 'date':
            // a date field from a pg database
            return "DATE '0'";
          case 'time':
          // a time field from a pg database
          case 'timetz':
            // a time field with timezone from a pg database
            return "TIME '0'";
          case 'timestamp':
          // a timestamp field from a pg database
          case 'timestamptz':
            // a timestamp field with timezone from a pg database
            return "TIMESTAMP '0'";
          default:
            // an unanticipated field type
            return "'0'";
        }
    }
  }
  else {
    if ($nulls) {
      switch (strtolower(trim($field_type))) {
        case 'bool':
        // a boolean from a pg database
        case 'boolean':
          // a generic type
          return "NULL";
        case 'int2':
        // a small integer from a pg database
        case 'int4':
        // an integer from a pg database
        case 'int8':
        // a long integer from a pg database
        case 'integer':
          // a generic type
          return "NULL";
        case 'numeric':
        // a "numeric" real number field from a pg database
        case 'float4':
        // a single precision real number field from a pg database
        case 'float8':
        // a double precision real number field from a pg database
        case 'real':
          // a generic type
          return "NULL";
        case 'text':
        // a text field from a pg database
        case 'varchar':
        // a varchar field from a pg database
        case 'bpchar':
          // a bpchar field from a pg database
          return "NULL";
        case 'date':
          // a date field from a pg database
          return "NULL";
        case 'time':
        // a time field from a pg database
        case 'timetz':
          // a time field with timezone from a pg database
          return "NULL";
        case 'timestamp':
        // a timestamp field from a pg database
        case 'timestamptz':
          // a timestamp field with timezone from a pg database
          return "NULL";
        default:
          // an unanticipated field type
          return "NULL";
      }
    }
    else {
      switch (strtolower(trim($field_type))) {
        case 'bool':
        // a boolean from a pg database
        case 'boolean':
          // a generic type
          return "FALSE";
        case 'int2':
        // a small integer from a pg database
        case 'int4':
        // an integer from a pg database
        case 'int8':
        // a long integer from a pg database
        case 'integer':
          // a generic type
          return "0";
        case 'numeric':
        // a "numeric" real number field from a pg database
        case 'float4':
        // a single precision real number field from a pg database
        case 'float8':
        // a double precision real number field from a pg database
        case 'real':
          // a generic type
          return "0";
        case 'text':
        // a text field from a pg database
        case 'varchar':
        // a varchar field from a pg database
        case 'bpchar':
          // a bpchar field from a pg database
          return "'FALSE'";
        case 'date':
          // a date field from a pg database
          return "DATE '0'";
        case 'time':
        // a time field from a pg database
        case 'timetz':
          // a time field with timezone from a pg database
          return "TIME '0'";
        case 'timestamp':
        // a timestamp field from a pg database
        case 'timestamptz':
          // a timestamp field with timezone from a pg database
          return "TIMESTAMP '0'";
        default:
          // an unanticipated field type
          return "'0'";
      }
    }
  }
}

/*
 * creates the correct string out of an integer variable value $val
 * to put inside VALUES() of a SQL INSERT statement
 * when the target field is of type $field_type
 * default is that nulls coming in map to nulls going out
 * should be universal - only tested for pgSQL though
 *
 * $val is a integer typed variable
 * $field_type is a string
 * returns a string with the necessary delimiters
 */
function fetchit7_integer_to_sql_values_str($val, $field_type = 'integer', $nulls = TRUE) {
  //debug($val,'$val');
  //debug($field_type,'$field_type');
  if (isset($val)) {// it likely will be by this point
    $intval = (int)$val;
    // just to be safe
    switch ($intval) {
      case 0:
        switch (strtolower(trim($field_type))) {
          case 'bool':
          // a boolean from a pg database
          case 'boolean':
            // a generic type
            return "FALSE";
          case 'int2':
          // a small integer from a pg database
          case 'int4':
          // an integer from a pg database
          case 'int8':
          // a long integer from a pg database
          case 'integer':
            // a generic type
            return "0";
          case 'numeric':
          // a "numeric" real number field from a pg database
          case 'float4':
          // a single precision real number field from a pg database
          case 'float8':
          // a double precision real number field from a pg database
          case 'real':
            // a generic type
            return "0";
          case 'text':
          // a text field from a pg database
          case 'varchar':
          // a varchar field from a pg database
          case 'bpchar':
            // a bpchar field from a pg database
            return "'0'";
          case 'date':
            // a date field from a pg database
            return "DATE '0'";
          case 'time':
          // a time field from a pg database
          case 'timetz':
            // a time field with timezone from a pg database
            return "TIME '0'";
          case 'timestamp':
          // a timestamp field from a pg database
          case 'timestamptz':
            // a timestamp field with timezone from a pg database
            return "TIMESTAMP '0'";
          default:
            // an unanticipated field type
            return "'0'";
        }
      default:
        switch (strtolower(trim($field_type))) {
          case 'bool':
          // a boolean from a pg database
          case 'boolean':
            // a generic type
            return "TRUE";
          case 'int2':
          // a small integer from a pg database
          case 'int4':
          // an integer from a pg database
          case 'int8':
          // a long integer from a pg database
          case 'integer':
            // a generic type
            return print_r($intval, TRUE);
          case 'numeric':
          // a "numeric" real number field from a pg database
          case 'float4':
          // a single precision real number field from a pg database
          case 'float8':
          // a double precision real number field from a pg database
          case 'real':
            // a generic type
            return print_r($intval, TRUE);
          case 'text':
          // a text field from a pg database
          case 'varchar':
          // a varchar field from a pg database
          case 'bpchar':
            // a bpchar field from a pg database
            return "'" . print_r($intval, TRUE) . "'";
          case 'date':
            // a date field from a pg database
            return "DATE '" . print_r($intval, TRUE) . "'";
          case 'time':
          // a time field from a pg database
          case 'timetz':
            // a time field with timezone from a pg database
            return "TIME '" . print_r($intval, TRUE) . "'";
          case 'timestamp':
          // a timestamp field from a pg database
          case 'timestamptz':
            // a timestamp field with timezone from a pg database
            return "TIMESTAMP '" . print_r($intval, TRUE) . "'";
          default:
            // an unanticipated field type
            return "'" . print_r($intval, TRUE) . "'";
        }
    }
  }
  else {
    if ($nulls) {
      switch (strtolower(trim($field_type))) {
        case 'bool':
        // a boolean from a pg database
        case 'boolean':
          // a generic type
          return "NULL";
        case 'int2':
        // a small integer from a pg database
        case 'int4':
        // an integer from a pg database
        case 'int8':
        // a long integer from a pg database
        case 'integer':
          // a generic type
          return "NULL";
        case 'numeric':
        // a "numeric" real number field from a pg database
        case 'float4':
        // a single precision real number field from a pg database
        case 'float8':
        // a double precision real number field from a pg database
        case 'real':
          // a generic type
          return "NULL";
        case 'text':
        // a text field from a pg database
        case 'varchar':
        // a varchar field from a pg database
        case 'bpchar':
          // a bpchar field from a pg database
          return "NULL";
        case 'date':
          // a date field from a pg database
          return "NULL";
        case 'time':
        // a time field from a pg database
        case 'timetz':
          // a time field with timezone from a pg database
          return "NULL";
        case 'timestamp':
        // a timestamp field from a pg database
        case 'timestamptz':
          // a timestamp field with timezone from a pg database
          return "NULL";
        default:
          // an unanticipated field type
          return "NULL";
      }
    }
    else {
      switch (strtolower(trim($field_type))) {
        case 'bool':
        // a boolean from a pg database
        case 'boolean':
          // a generic type
          return "FALSE";
        case 'int2':
        // a small integer from a pg database
        case 'int4':
        // an integer from a pg database
        case 'int8':
        // a long integer from a pg database
        case 'integer':
          // a generic type
          return "0";
        case 'numeric':
        // a "numeric" real number field from a pg database
        case 'float4':
        // a single precision real number field from a pg database
        case 'float8':
        // a double precision real number field from a pg database
        case 'real':
          // a generic type
          return "0";
        case 'text':
        // a text field from a pg database
        case 'varchar':
        // a varchar field from a pg database
        case 'bpchar':
          // a bpchar field from a pg database
          return "'FALSE'";
        case 'date':
          // a date field from a pg database
          return "DATE '0'";
        case 'time':
        // a time field from a pg database
        case 'timetz':
          // a time field with timezone from a pg database
          return "TIME '0'";
        case 'timestamp':
        // a timestamp field from a pg database
        case 'timestamptz':
          // a timestamp field with timezone from a pg database
          return "TIMESTAMP '0'";
        default:
          // an unanticipated field type
          return "'0'";
      }
    }
  }
}

function fetchit_booleantointeger($val) {
	switch (strtolower($val)) {
	case "t":
	case "true":
	case "yes":
	case "1":
	case "on":
		return 1;
	case "f":
	case "false":
	case "no":
	case "0":
	case "off":
		return 0;
	}
	switch ($val) {
	case 0:
		return 0;
	default:
		return 1;
	}
}

function fetchit7_get_control_list() {
	return array(// add 1 to make array_search() easier.  subtract 1 later...
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
}

function fetchit7_get_control_list_string() {
  $control_list = fetchit7_get_control_list();
  $control_list_string = '';
  $count = 0;
  foreach ($control_list as $control_string) {
    if ($count)
      $control_list_string .= ', ';
    $control_list_string .= $control_string;
    ++$count;
  }
  return $control_list_string;
}

function fetchit7_strip_special_char_default($val) {
  $newval=$val;
  if (strlen($val)) {
    switch ($val[0]) {
      case '-':
      case '*':
      case '+':
        $newval = substr($val, 1);
        break;
      default:
    }
  }
  return $newval;
}

function fetchit7_strip_special_char_allnone($val) {
	$newval=$val;
	if (strlen($val)) {
		switch ($val[0]) {
			case '-':
			case '*':
			case '+':
				$newval = substr($val, 1);
				break;
			default:
		}
	}
	return $newval;
}

function fetchit7_get_special_char_default($val) {
  $special_char = '';
  if (strlen($val)) {
    switch ($val[0]) {
      case '-':
      case '*':
      case '+':
        $special_char = $val[0];
        break;
      default:
    }
  }
  return $special_char;
}

function fetchit7_get_special_char_allnone($val) {
	$special_char = '';
	if (strlen($val)) {
		switch ($val[0]) {
			case '-':
			case '*':
			case '+':
				$special_char = $val[0];
				break;
			default:
		}
	}
	return $special_char;
}
