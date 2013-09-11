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
  return $forms;
}

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
