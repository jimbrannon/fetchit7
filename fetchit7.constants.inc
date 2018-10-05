<?php
// fetchit7 temporary location for various content type option values
//   that are not part of the actual content types yet
// going forward the code will have to do some "if exist" type constructions
//   to make this proceed cleanly
define("FETCHIT7_OPTIONS_TEXTBOX_SIZE",7); // default textbox size on some types like booleans and ints when it can't be estimated another way
define("FETCHIT7_OPTIONS_TEXTBOX_NUMBER_SIZE",9); // default textbox size on real number types when it can't be estimated another way
define("FETCHIT7_OPTIONS_TEXTBOX_TEXT_SIZE",20); // default textbox size on text or unknown types when it can't be estimated another way
define("FETCHIT7_OPTIONS_TEXTBOX_MAXLENGTH",250); // max length of string input into textbox
define("FETCHIT7_OPTIONS_TABLE_SUBMIT_SUCCESS",1);
define("FETCHIT7_OPTIONS_PARAM_SUBMIT_SUCCESS",1);
define("FETCHIT7_OPTIONS_EGRID_SUBMIT_SUCCESS",1);
define("FETCHIT7_OPTIONS_APPND_SUBMIT_SUCCESS",1);

// fetchit7 content type names - used in node table "type" field, field instance "bundle" field
define("FETCHIT7_CONTENT_NAME_TABLE","fetchit7_content_table");
define("FETCHIT7_CONTENT_NAME_PARAM","fetchit7_content_param");
define("FETCHIT7_CONTENT_NAME_EGRID","fetchit7_content_egrid");
define("FETCHIT7_CONTENT_NAME_APPND","fetchit7_content_appnd");

define("FETCHIT7_DBTYPE_NAME_PGSQL","postgresql");
define("FETCHIT7_DBTYPE_LABEL_PGSQL","PostgreSQL");
define("FETCHIT7_DBTYPE_NAME_MYSQL","mysql");
define("FETCHIT7_DBTYPE_LABEL_MYSQL","MySQL");
define("FETCHIT7_DBTYPE_NAME_MSSQL","sqlserver");
define("FETCHIT7_DBTYPE_LABEL_MSSQL","MS SQL Server");

// these are the fetchit2 parameter form control types - as numbers
define("F_DR_CONTROL_TEXTBOX",0);
define("F_DR_CONTROL_DATESEL",1);
define("F_DR_CONTROL_DROPDOWN_SINGLE",2);
define("F_DR_CONTROL_RADIOBTN",3);
define("F_DR_CONTROL_CHECKBOXES",4);
define("F_DR_CONTROL_DROPDOWN_MULTI",5);
define("F_DR_CONTROL_CHECKBOX",6);
define("F_DR_CONTROL_DATETIMESEL",7);
define("F_DR_CONTROL_POPUPDATE",8);

// these are the fetchit7 parameter form control types - as string tokens
define("FETCHIT7_CONTROLTYPE_NAME_TEXTBOX","textbox"); //define("F_DR_CONTROL_TEXTBOX",0);
define("FETCHIT7_CONTROLTYPE_NAME_DATESEL","date_select"); //define("F_DR_CONTROL_DATESEL",1);
define("FETCHIT7_CONTROLTYPE_NAME_DROPDOWN_SINGLE","dropdown_single"); //define("F_DR_CONTROL_DROPDOWN_SINGLE",2);
define("FETCHIT7_CONTROLTYPE_NAME_RADIOBTN","radio_buttons"); //define("F_DR_CONTROL_RADIOBTN",3);
define("FETCHIT7_CONTROLTYPE_NAME_CHECKBOXES","checkboxes"); //define("F_DR_CONTROL_CHECKBOXES",4);
define("FETCHIT7_CONTROLTYPE_NAME_DROPDOWN_MULTI","dropdown_multiple"); //define("F_DR_CONTROL_DROPDOWN_MULTI",5);
define("FETCHIT7_CONTROLTYPE_NAME_CHECKBOX","checkbox"); //define("F_DR_CONTROL_CHECKBOX",6);
define("FETCHIT7_CONTROLTYPE_NAME_DATETIMESEL","datetime_select"); //define("F_DR_CONTROL_DATETIMESEL",7);
define("FETCHIT7_CONTROLTYPE_NAME_POPUPDATE","date_popup"); //define("F_DR_CONTROL_POPUPDATE",8);

define("F_DR_TEXTBOX_TEXT",0);
define("F_DR_TEXTBOX_NUMBER",1);
define("F_DR_TEXTBOX_DATE",2);
define("FETCHIT7_CONTROLTYPE_TEXTBOXTYPE_NAME_TEXT","text"); //define("F_DR_TEXTBOX_TEXT",0);
define("FETCHIT7_CONTROLTYPE_TEXTBOXTYPE_NAME_NUMBER","number"); //define("F_DR_TEXTBOX_NUMBER",1);
define("FETCHIT7_CONTROLTYPE_TEXTBOXTYPE_NAME_DATE","date"); //define("F_DR_TEXTBOX_DATE",2);

// these are the array keys used for field parameters
define("F_DR_FIELD_NAME_STR","name");
define("F_DR_FIELD_TITLE_STR","title");
define("F_DR_FIELD_DESC_STR","desc");
define("F_DR_FIELD_INPUT_TYPE_STR","input_type");
define("F_DR_FIELD_DEFAULT_STR","default");
define("FETCHIT7_ALLNONE_SPECIAL_CHAR_STR","allnone_specialcharacter");
define("F_DR_FIELD_DISABLED_STR","disabled");
define("F_DR_FIELD_REQUIRED_STR","required");
define("F_DR_FIELD_VALIDATE_STR","validate");
define("F_DR_FIELD_SUBMIT_STR","submit");
define("F_DR_FIELD_PARAMETER_STR","input_parm");
define("FETCHIT7_FIELD_WEIGHT_STR","weight");
define("FETCHIT7_FIELD_HIDDEN_STR","hidden");

// these are the array keys used for field input list table parameters
define("F_DR_FIELD_LIST_TBL_STR","list_tbl");
//define("F_DR_FIELD_LIST_TBL_CHAR_STR","list_tbl_char");
define("FETCHIT7_DEFAULT_SPECIAL_CHAR_STR","default_specialcharacter");
define("FETCHIT7_FIELD_LIST_NDX_STR","list_ndx"); //define("F_DR_LIST_TBL_NDX_FLD","stored");
define("FETCHIT7_FIELD_LIST_DSP_STR","list_dsp"); //define("F_DR_LIST_TBL_DSP_FLD","displayed");
define("F_DR_FIELD_LIST_UID_STR","list_uid"); //define("F_DR_LIST_TBL_UID_FLD","drupal_userid");

// these are the array keys used for field data output table parameters
define("F_DR_FIELD_DATA_TBL_STR","data_tbl");
define("F_DR_FIELD_DATA_KEY_STR","data_key"); //define("F_DR_DATA_TBL_NDX_FLD","stored");
define("F_DR_FIELD_DATA_UID_STR","data_uid"); //define("F_DR_DATA_TBL_UID_FLD","drupal_userid");

//these are some default field input list table parameter values
define("F_DR_LIST_TBL_SPECIAL_CHAR","#");
define("F_DR_LIST_TBL_NDX_FLD","stored");
define("F_DR_LIST_TBL_DSP_FLD","displayed");
define("F_DR_LIST_TBL_UID_FLD","drupal_userid");

//these are some default field data output table parameter values
define("F_DR_DATA_TBL_NDX_FLD","stored");
define("F_DR_DATA_TBL_UID_FLD","drupal_userid");

define("F_DR_FORM_ELE_DEF","default");
define("F_DR_FORM_ERROR_DEF","<br><b>Error: Form could not be created.</b><br>");

define("FETCHIT7_FIELD_DEFAULT_PREVIOUS","FETCHIT7_DEFAULT_PREVIOUS");
define("FETCHIT7_FIELD_DEFAULT_NONE","FETCHIT7_DEFAULT_NONE");
define("FETCHIT7_FIELD_DEFAULT_BLANK","FETCHIT7_DEFAULT_BLANK");
define("FETCHIT7_FIELD_DEFAULT_FIRST","FETCHIT7_DEFAULT_FIRST");
define("FETCHIT7_FIELD_DEFAULT_ALL","FETCHIT7_DEFAULT_ALL");
define("FETCHIT7_FIELD_DEFAULT_TRUE","FETCHIT7_DEFAULT_TRUE");
define("FETCHIT7_FIELD_DEFAULT_FALSE","FETCHIT7_DEFAULT_FALSE");

?>
