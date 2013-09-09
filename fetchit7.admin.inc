<?php

/**
 * @file
 * Adminsitration page callbacks for the fetchit7 module
 */
 
 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_settings() {
	// create a form with the basic fetchit configuration options
	// menu root name, menu root name title, which "db connected" form types are enabled
	// note this used to be a CCK in fetchit2 (drupal 6)
}

function fetchit7_admin_reset() {
	// not sure this is actually needed anymore...but wanted to try having two callbacks in this inc file
}
