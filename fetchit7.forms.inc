<?php

/**
 * @file
 * Forms page callbacks for the fetchit7 module
 */

/**
 * Implements hook_forms().
 * 
 * Allows the fetchit7 form to be repeated multiple times on a page.
 * See http://drupal.org/node/1970846.
 */
function fetchit7_forms($form_id, $args) {
  $forms = array();
  if (substr($form_id, 0, 18) == 'fetchit7_xxx_form_') {
    $forms[$form_id] = array('callback' => 'fetchit7_xxx_form');
  }
  if (substr($form_id, 0, 18) == 'fetchit7_tbl_form_') {
    $forms[$form_id] = array('callback' => 'fetchit7_tbl_form');
  }
  return $forms;
}



//===================================
// fetchit7 html table form content
//===================================

/**
  * Form builder.  Display fetchit7 html table content.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_tbl_form($form, &$form_state, $args) {
	// form builder function

	// get args
	print_r("hello");
	print_r($form);
	print_r($form_state); //['build_info']['args'][0];
	print_r($args);
		
	// text element
	$myform['fetchit7_tbl_text'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_tbl_text','fetchit7_tbl_text_default_value'),
		'#title' => t('Enter some text'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$myform['#submit'][] = 'fetchit7_tbl_form_submit';
	return system_settings_form($myform);
	
}

/**
 * Process form validation
 */
function fetchit7_tbl_form_validate ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_tbl_form_validate");
}

/**
 * Process form submission
 */
function fetchit7_tbl_form_submit ($form, &$form_state) {
	// pop a log message
	watchdog("fetchit7 module","entered fetchit7_tbl_form_submit");
	// reset the text
	if(!strlen(trim($string = $form_state['values']['fetchit7_tbl_text']))) $string = 'fetchit7_tbl_text_default_value';
	variable_set('fetchit7_tbl_text',$string);
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
