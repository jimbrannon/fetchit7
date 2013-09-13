<?php

/**
 * @file
 * Forms page callbacks for the fetchit7 module
 */




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
//function fetchit7_table_form() {
	// form builder function
	//print_r($form);
	$unique_id = $form_state['build_info']['form_id'];
	print_r($unique_id);
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
		'#value' => 'submit',
		'#id' => $unique_id,
	);

	$form['#id'] = $unique_id;
	$form['#submit'][] = 'fetchit7_table_form_submit';

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
