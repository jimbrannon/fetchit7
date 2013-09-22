<?php

/**
 * @file
 * Adminisitration page callbacks for the fetchit7 module
 */
 
include_once(drupal_get_path('module', 'fetchit7').'/fetchit7.constants.inc'); 
 
 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_settings() {
	// create a form with the basic fetchit configuration options
	// menu root name, menu root name title, which "db connected" form types exist
	// the "db connected" form types are new content types (node types) created and populated by
	// the fetchit7.install file

	// set the root menu machine name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_name','fetchit7_nodes');
	$form['fetchit7_root_menu_name'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_name','fetchit7_nodes'),
		'#title' => t('Enter the fetchit7 root menu internal machine name (generally, leave this alone...)'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	// set the root menu displayed name...perhaps allow the user to change this in a text box
	//variable_set('fetchit7_root_menu_title','fetchit7 nodes');
	$form['fetchit7_root_menu_title'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_root_menu_title','fetchit7 nodes'),
		'#title' => t('Enter the fetchit7 root menu displayed name'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
		
	// set whether the root menu is displayed...perhaps allow the user to change this with a check box
	//variable_set('fetchit7_root_menu_displayed',FALSE);
	$form['fetchit7_root_menu_displayed'] = array(
		'#type' => 'checkbox',
		'#default_value' => variable_get('fetchit7_root_menu_displayed',FALSE),
		'#title' => t('Display the fetchit7 nodes menu'),
		'#description' => t('Only select this if you want a fetchit7 menu with submenus for all the page content to be displayed by Drupal.'),
	);

	// get the fetchit7 content types from the fetchit7_content_types table and display them
	// this does not do much yet (maybe enables/disables them as a group?)
	$d7tablename = 'fetchit7_content_types';
	$d7returnfieldname = array('ndx','title','enabled');
	$query1 = db_select($d7tablename, 'n')
	->fields('n', $d7returnfieldname);
	$result1 = $query1->execute();
	$options = array();
	$enabled = array();
	// create the array of checkboxes to mark, enabled[]
	foreach ($result1 as $row1) {
		$options[$row1->ndx] = $row1->title;
		if ($row1->enabled) $enabled[] = $row1->ndx;
	}
	//
	$form['fetchit7_content_types'] = array(
		'#type' => 'checkboxes',
		'#title' => t('Available fetchit7 content types'),
		'#options' => $options,
		'#default_value' => $enabled,
		'#description' => t('Check the content types where created forms should be enabled by default.'),
	);
	
	$form['#submit'][] = 'fetchit7_admin_settings_submit';
	return system_settings_form($form);
	
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_settings_submit ($form, $form_state) {
	// reset the fetchit7 root menu name
	if(!strlen(trim($string = $form_state['values']['fetchit7_root_menu_name']))) $string = 'fetchit7_nodes';
	variable_set('fetchit7_root_menu_name',$string);
	// reset the fetchit7 root menu label
	if(!strlen(trim($string = $form_state['values']['fetchit7_root_menu_title']))) $string = 'fetchit7 nodes';
	variable_set('fetchit7_root_menu_title',$string);
	// reset the fetchit7 root menu flag
	variable_set('fetchit7_root_menu_displayed',$form_state['values']['fetchit7_root_menu_displayed']);
	// update the fetchit7 content types enabled flags in the fetchit7_content_types table
	foreach ($form_state['values']['fetchit7_content_types'] as $key => $value) {
		$num_updated = db_update('fetchit7_content_types')
			->fields(array('enabled' => $value))
			->condition('ndx',$key,'=')
			->execute();
	}
}

/**
  * Form builder.  Configure fetchit7 database settings.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_dbsettings() {
	// these are the default database server connection settings
	// this will allow the fetchit7 user to only need to enter these once
	// i love my users :-)
	
	// set the default database server host name
	//variable_set('fetchit7_default_db_host','localhost');
	$form['fetchit7_default_db_host'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_host','localhost'),
		'#title' => t('Enter the default database server host name to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
		
	// set the default database server type
	//variable_set('fetchit7_default_db_type','PostgreSQL');
	$options = array();
	$options = array(0 => t('PostgreSQL'), 1 => t('MySQL'), 2 => t('SQL Server'));
	$form['fetchit7_default_db_type'] = array(
    '#type' => 'radios',
    '#title' => t('Select the default database server type to use for database connections'),
    '#default_value' => 0,
    '#options' => $options,
    '#description' => t('These are the database server types supported by fetchit7.'),
		'#required' => TRUE,
  );

	// set the default database server port number
	//variable_set('fetchit7_default_db_port', 5432);
	$form['fetchit7_default_db_port'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_port', 5432),
		'#title' => t('Enter the default database server port number to use for database connections'),
		'#size' => 10,
		'#maxlength' => 20,
		'#required' => TRUE,
	);
	
	// set the default database server database name
	//variable_set('fetchit7_default_db_name', 'default_db_name');
	$form['fetchit7_default_db_name'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_name', 'default_db_name'),
		'#title' => t('Enter the default database name to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	// set the default database server login name
	//variable_set('fetchit7_default_db_login', 'default_db_login');
	$form['fetchit7_default_db_login'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_login', 'default_db_login'),
		'#title' => t('Enter the default database login to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	// set the default database server password
	//variable_set('fetchit7_default_db_password', 'default_db_password');
	$form['fetchit7_default_db_password'] = array(
		'#type' => 'textfield',
		'#default_value' => variable_get('fetchit7_default_db_password', 'default_db_password'),
		'#title' => t('Enter the default database password to use for database connections'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);

	$form['#submit'][] = 'fetchit7_admin_dbsettings_submit';
	return system_settings_form($form);
	
}


/**
 * Process fetchit7 default database settings submission
 */
function fetchit7_admin_dbsettings_submit ($form, $form_state) {
	//if(!strlen(trim($db_host = $form_state['values']['fetchit7_default_db_host']))) $db_host = 'localhost';
	$db_host = $form_state['values']['fetchit7_default_db_host'];
	//if(($db_type = $form_state['values']['fetchit7_default_db_type'])>2) $db_type = 0;
	$db_type = $form_state['values']['fetchit7_default_db_type'];
	//if(!($db_port = $form_state['values']['fetchit7_default_db_port'])) $db_port = 5432;
	$db_port = $form_state['values']['fetchit7_default_db_port'];
	$db_name = $form_state['values']['fetchit7_default_db_name'];
	$db_login = $form_state['values']['fetchit7_default_db_login'];
	$db_password = $form_state['values']['fetchit7_default_db_password'];
	// update the fetchit7 content types db settings in the fetchit7_content_types table
	$num_updated = db_update('fetchit7_content_types')
		->fields(array(
			'db_host' => $db_host,
			'db_type' => $db_type,
			'db_port' => $db_port,
			'db_name' => $db_name,
			'db_login' => $db_login,
			'db_password' => $db_password,
			)
		)
		->execute();
}


 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  * 
 * Backup/restore page callbacks for the fetchit7 admin
 * 
 * the objective is to create a backup and restore mechanism for fetchit7 content - the node content types and associated fields -
 * so that it can be moved from site to site, or recovered after a fetchit uninstall/install cycle
 * 
 * the idea on the backup side is to simply convert the existing drupal 7 schema to flat tables in the host database, one per content type
 * 
 * on the backup side, should be doable using the instance info functions to get the fields, and the field info functions to get the field types
 * 
 * on the restore side, more care has to be taken, because the schema might be different - as in a uninstall/install cycle on a fetchit upgrade
 * therefore new records for each type will be created with the default values, then those fields which still exist will be replaced with the saved values.
 * 
 * basic and straight-forward, but should work for most circumstances site developers will encounter
  */
  
function fetchit7_admin_backupcontent() {
	
	$form = array();
	
	// text element
	$form['fetchit7_admin_backupcontent_name'] = array(
		'#type' => 'textfield',
		'#default_value' => 'default',
		'#title' => t('Enter a name for this backup.  Warning: any previous backup with this same name will be overwritten.'),
		'#size' => 30,
		'#maxlength' => 60,
		'#required' => TRUE,
	);
	
	$form['fetchit7_admin_backupcontent_submit'] = array(
		'#type' => 'submit',
		'#value' => 'Backup All Fetchit7 Content',
	);
	

	//$form['#submit'][] = 'fetchit7_admin_backupcontent_submit';
	//return system_settings_form($form);
	
	return $form;
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_backupcontent_submit ($form, $form_state) {
	// pop a log message
	//watchdog("fetchit7 module","entered fetchit7_admin_backupcontent_submit");
	// get the user entered string containing the backup name
	if(!strlen(trim($string = $form_state['values']['fetchit7_admin_backupcontent_name']))) $string = 'default';
	// fetchit7 content types
	// TODO: should loop through all fetchit7 content types - search by name? - rather than have them hardcoded here
	$content_types = array('fetchit7_content_table','fetchit7_content_param');
	$contenttypecount = 0;
	$contentcount = 0;
	foreach ($content_types as $content_type) {
		$tablename = $content_type.'_backup_'.$string;
		if($newrecords = fetchit7_content_backuptable_create($content_type,$tablename)) {
			++$contenttypecount;
			$contentcount = $contentcount + $newrecords;
		}
	}
	drupal_set_message(t("Successfully saved $contenttypecount content types, $contentcount records total."),'status');

}

function fetchit7_content_backuptable_create($type,$tablename) {
	//=============================================================
	// create the table, deleting any existing table with that name
	//=============================================================
	$nodetablefields = array();
	$entityfields = array();
	$fieldstrings = array();
	$fieldstrings[] = 'ndx int PRIMARY KEY';
	$query  = fetchit7_maketablequery_create($tablename,$fieldstrings);
	if(!($result = db_query($query))) return FALSE;
	$query = "DROP TABLE $tablename;";
	if(!($result = db_query($query))) return FALSE;
	// the node table fields
	$fieldstrings = array();
	$fieldstrings[] = 'nid int PRIMARY KEY';
	$nodetablefields[] = 'nid';
	$fieldstrings[] = 'type varchar (255)';
	$nodetablefields[] = 'type';
	$fieldstrings[] = 'title varchar (255)';
	$nodetablefields[] = 'title';
	// get the fetchit7 installed fields for this type (i.e. instances)
	$instances = field_info_instances('node',$type);
	foreach ($instances as $instance_name => $instance) {
		$field_name = $instance['field_name'];
		$entityfields[] = $field_name;
		$field = field_info_field($field_name);
		$field_type = $field['columns']['value']['type'];
		$field_types[$field_name] = $field_type;
		if ($field_type == 'varchar') {
			//actually, had to make it as big as possible (255) so that we can cram
			// as many comma delimited values in there as possible because of
			// multiple valued fields in the param input content type
			//$field_type .= ' ('.$field['columns']['value']['length'].')';
			$field_type .= ' (255)';
		}
		$fieldstrings[] = $field_name.' '.$field_type;
	}
	//debug($field_types,'$field_types');
	$query  = fetchit7_maketablequery_create($tablename,$fieldstrings);
	if(!($result = db_query($query))) return FALSE;

	//=============================================================
	// populate the new table based on the output of EntityFieldQuery
	// and the node table
	//=============================================================
	$query = new EntityFieldQuery();
	$query
		->entityCondition('entity_type', 'node')
		->entityCondition('bundle', $type)
		->propertyCondition('status', 1)
		->propertyOrderBy('created', 'DESC')
		//->fieldCondition('field_us_state', 'value', array('CT', 'NJ', 'NY'))
		//->range(0,10)
		;
	$result = $query->execute();
	$inserted_count = 0;
	if (!empty($result['node'])) {
		// get the nids
		$entityfieldnids = array_keys($result['node']);
		$entityfieldnodes = node_load_multiple($entityfieldnids);
		//$entityfieldnodes_form = node_view_multiple($entityfieldnodes, 'full');
		//now get the nids in the node table for this type
		$sql = 'SELECT * FROM {node} n WHERE n.type = :type';
		$result = db_query($sql, array(':type' => $type));
		// loop through the records and get the field values from both object arrays
		// to create an insert query
		// loop over the node table list - this should be the same as the entityfield list (right?)
		// the $entityfieldnodes object array is keyed by the nid, so it will be easy to get the object we need
		foreach ($result as $row) {
			$data = array();
			foreach ($nodetablefields as $nodetablefield) {
				$data[$nodetablefield] = $row->$nodetablefield;
			}
			$datnode = $entityfieldnodes[$row->nid];
			foreach ($entityfields as $entityfield) {
				$datfield = $datnode->$entityfield;
				if(array_count_values($dozlanguagekeys = array_keys($datfield))) {
					// use the first language key, whatever it is - usually it is 'und'
					$datlanguagekey = $dozlanguagekeys[0];
					//TODO: need to update backup function to find a better way (something non-text based and not assuming varchars) to handle multiple valued fields
					if (array_count_values($dozdatakeys = array_keys($datfield[$datlanguagekey]))) {
						$datcounter = 0;
						foreach ($dozdatakeys as $datdatakey) {
							if ($datcounter) {
								// it HAS to be a varchar or we can do nothing more !! (for now...)
								// param input content types only have multiple values for varchars, so we are good. (for now...)
								if ($field_types[$entityfield] == 'varchar') {
									// it's a varchar! chunk more data in that puppy
									// we are assuming that we set the varchar size as high as it goes (255) on this field
									// there better be no commas in those strings or we are screwed!!
									// how much room do we have left?
									$daroomleft = 255 - strlen($data[$entityfield]);
									if (strlen($datfield[$datlanguagekey][$datdatakey]['value']) < $daroomleft) {
										$data[$entityfield] .= ',';
										$data[$entityfield] .= $datfield[$datlanguagekey][$datdatakey]['value'];
									}
								}
							} else {
								// $datdatakey SHOULD be zero, but just to be careful....
								$data[$entityfield] = $datfield[$datlanguagekey][$datdatakey]['value'];
								// most times...and we are done...but sometimes...
							}
							++$datcounter;
						}
					}
				}
			}
			//now do the insert
			$insertednid = db_insert($tablename) // Table name no longer needs {}
				->fields($data)
				->execute();
			++$inserted_count;
		}
	}
	
	return $inserted_count;
}


/**
 * function to create a MAKE TABLE query
 */
function fetchit7_maketablequery_create($tablename,$fieldstrings) {
	$query  = 'CREATE TABLE IF NOT EXISTS ';
	$query .= $tablename;
	$query .= ' (';
	foreach ($fieldstrings as $key=>$fieldstring) {
		if ($key) $query .= ',';
		$query .= $fieldstring;
	}
	$query .= ');';
	
	return $query; 
}

 /**
  * Form builder.  Configure fetchit7.
  * 
  * @ingroup forms
  * @see system_settings_form().
  */
function fetchit7_admin_restorecontent() {
	global $databases;
	$form = array();
	// get the fetchit7 content backup tables to choose from. 
	// first get the database name
  $db_name = $databases['default']['default']['database'];
	// now query for table names that meet the pattern
	$sql = 'SHOW TABLES LIKE \'fetchit7_content_%_backup_%\';';
	$result = db_query($sql);
	$fieldname = 'Tables_in_'.$db_name.' (fetchit7_content_%_backup_%)';
	$options = array();
	foreach ($result as $row) {
		$options[] = $row->$fieldname;
	}
	# put up a select list
	 $form['fetchit7_content_backup_table_select'] = array(
       '#type' => 'select',
       '#title' => t('fetchit7 content backup tables'),
       '#options' => $options,
       '#default_value' => 0,
       '#description' => t('Choose a fetchit7 content backup table to restore.'),
   );
	
	$form['fetchit7_admin_restorecontent_submit'] = array(
		'#type' => 'submit',
		'#value' => 'Restore fetchit7 content',
	);

	//$form['#submit'][] = 'fetchit7_admin_restorecontent_submit';
	//return system_settings_form($form);
		
	return $form;
}

/**
 * Process fetchit7 settings form submission
 */
function fetchit7_admin_restorecontent_submit ($form, $form_state) {
	// get the backup table name
	$options = $form['fetchit7_content_backup_table_select']['#options'];
	$tablename = $options[$form_state['values']['fetchit7_content_backup_table_select']];
	// ugly
	switch (substr($tablename,17,5)) {
		case 'table':
			$content_type = 'fetchit7_content_table';
			break;
		case 'param':
			$content_type = 'fetchit7_content_param';
			break;
		default:
			drupal_set_message(t("Error processing fetchit7 backup table $tablename"),'warning');
			return FALSE;
	}
	$newrecords = fetchit7_content_backuptable_restore($content_type,$tablename);
	drupal_set_message(t("Successfully restored $newrecords records of type $content_type from table $tablename."),'status');
}

function fetchit7_content_backuptable_restore($type,$tablename) {
	// 1. open the source table and get the field names
	// 2. get the content type schema field names and find the ones that are in teh backup table
	// 3. loop through the source table records
	// 3.5 create a content type node object
	// 4. loop through the content type schema field names
	// 5. if the field name matches a field from the source table
	//    then add the source table value to the object 
	// 6. save the object to the new entity schema
	// this skips fields in teh backup talbe that are not in the 
	// new schema definition
	// it also allows the missing fiields to get defaults defined in the content schema
	// it likely will get a new nid, to the site developer might have to
	// reconnect content to blocks, etc.

	// 1
	$query = "SHOW COLUMNS FROM $tablename;";
	if(!($result = db_query($query))) return FALSE;
	$count = 0;
	foreach ($result as $row) {
		$backuptablecolumns[] = $row->Field;
		++$count;
	}
	if (!$count) return FALSE;
	
	// 2A
	// these are definitely in the node table
	// but ignore nid because it will be assigned a new one when saved
	// and ignore type because we assume it is there and will be handled specifically
	$nodetablefields = array('title');
	$nodetablefieldsinbackuptable = array();
	foreach ($nodetablefields as $field_name) {
		if(in_array($field_name, $backuptablecolumns)) $nodetablefieldsinbackuptable[] = $field_name;
	}
	// 2B
	//$entityfields = array();
	$entityfieldsinbackuptable = array();
	$instances = field_info_instances('node',$type);
	foreach ($instances as $instance_name => $instance) {
		$field_name = $instance['field_name'];
		if(in_array($field_name, $backuptablecolumns)) $entityfieldsinbackuptable[] = $field_name;
		//$entityfields[] = $field_name;
	}
	// 3
	$query = db_select($tablename,'t')->fields('t',$backuptablecolumns);
	$result = $query->execute();
	$inserted_count = 0;
	foreach ($result as $row) {
		// 3.5
		$newNode = new stdClass();
		$newNode->type = $type;
		node_object_prepare($newNode);
		// 4
		foreach ($nodetablefieldsinbackuptable as $field_name) {
			// 5
			$newNode->$field_name = $row->$field_name;
		}
		// 4
		$dozexplodablefields = array('fetchit7_field_par_fld');
		foreach ($entityfieldsinbackuptable as $field_name) {
			// 5
			if (in_array(substr($field_name,0,22),$dozexplodablefields)) {
				$explodeit = true;
			} else {
				$explodeit = false;
			}
			$myarray = array();
			if ($explodeit) {
				$dozvalues = explode(",",$row->$field_name);
				$datvaluecounter = 0;
				foreach ($dozvalues as $datvalue) {
					$myarray['und'][$datvaluecounter]['value'] = $datvalue;
					++$datvaluecounter;
				}
			} else {
				$myarray['und'][0]['value'] = $row->$field_name;
			}
			$newNode->$field_name = $myarray;
		}
		// 6
		$new = node_save($newNode);
		++$inserted_count;
	}
	
	return $inserted_count;
}


/**
 * functions I might need...
 * http://drupal.stackexchange.com/questions/5951/is-there-a-drupal-api-function-to-get-the-list-of-nodes-by-type
 */
function node_load_by_type($type, $limit = 100, $offset = 0) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', $type)
    ->range($offset, $limit);
  $result = $query->execute();
  $nodes = array();
  if (isset($result['node'])) {
  	$nodes = node_load_multiple(array_keys($result['node']));
	}
	
	return $nodes;
}

function entity_load_by_type($entity_type, $bundle, $limit = 100, $offset = 0) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', $entity_type)
    ->entityCondition('bundle', $bundle)
    ->range($offset, $limit);
  $result = $query->execute();
  $entities = array();
  if (isset($result[$bundle])) {
  	$entities = entity_load($entity_type, array_keys($result[$bundle]));
	}
	
  return $entities;
}


