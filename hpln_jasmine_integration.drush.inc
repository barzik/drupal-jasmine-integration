<?php
/**
 * Created by JetBrains PhpStorm.
 * User: barzik
 * Date: 11/09/13
 * Time: 13:07

 */

/**
 * INSTALL
 * Move this drush script to drush's installation inside the commands/ directory
 *
 * USAGE
 * EXample: drush hpln_install_patch patchfile
 */

function drupal_jasmine_integration_drush_command() {
    return array(
        'hpln_run_jasmine' => array(
            'callback' => 'hpln_run_jasmine_drush_callback',
            'description' => 'drush command for running phantomjs with the jasmine integration tests suite'
        )
    );
}

/**
 * Implementation for hook_drush_help
 * @param $section
 * @return string
 */

function drupal_jasmine_integration_drush_help($section) {
    switch($section){
        case 'drush:hpln_run_jasmine':
            return "drush command for running phantomjs with the jasmine integration tests suite";
    }
}


/**
 * the drush command callback
 */
function hpln_run_jasmine_drush_callback() {

    // module must be installed, so lets make sure it is, else we exit.
    if (!module_exists('drupal_jasmine_integration')) {
        drush_die('module hpln jasmine integration must be activated', 1);
        return;
    }
    // phantomjs must be installed, so lets make sure it is, else we exit.
    exec("phantomjs -v", $out, $phantom_return);
    if($phantom_return != 0 ) {
        drush_die('pantomjs is not installed on this machine', 1);
        return;
    }

    //Creating several variables that I will use later on
    $phantom_session_file = file_directory_path().'/phantom_session_key.txt';
    $drupal_url = variable_get('hpln_portal_url_lb', '');
    $drupal_admin = variable_get('hpln_sso_admin_backdoor_hash', '');
    $jasmine_integration_module_path = drupal_get_path('module', 'drupal_jasmine_integration');

    //getting the manifest. we need it to know what URL to load
    $manifest_object = hpln_get_manifest();
    if($manifest_object == null)
        drush_die('No manifest file.', 1);

    /** running through the manifest object, if we don't need login, run it */
    drush_print("Running tests for anonymous users");
    drupal_jasmine_integration_run_all_tests(FALSE, $jasmine_integration_module_path, $manifest_object, $drupal_url,$phantom_session_file);

    /** deleting and creating the session. and than run all the tests that need log in */
    drupal_jasmine_integration_delete_session($phantom_session_file);
    drupal_jasmine_integration_create_session($phantom_session_file, $jasmine_integration_module_path, $drupal_url,$drupal_admin);
    drush_print("Running tests for logged-in users");
    drupal_jasmine_integration_run_all_tests(TRUE, $jasmine_integration_module_path, $manifest_object, $drupal_url,$phantom_session_file);

}

/**
 * deleting old session file
 * @param $phantom_session_file
 */
function drupal_jasmine_integration_delete_session($phantom_session_file) {
    if(file_exists($phantom_session_file)) {
        drush_print('deleting old session file...'.file_directory_path());
        unlink($phantom_session_file);
    }
}

/**
 * Creating session file.
 * @param $phantom_session_file
 * @param $drupal_url
 * @param $drupal_admin
 */

function drupal_jasmine_integration_create_session($phantom_session_file, $jasmine_integration_module_path, $drupal_url,$drupal_admin) {
    drush_print('Generating new admin session file');
    $session_creation = sprintf("phantomjs --cookies-file=%s %s/js/drupal_jasmine_get_admin_cookie.js %s/?destination=adminbackdoor key=%s",$phantom_session_file, $jasmine_integration_module_path, $drupal_url, $drupal_admin);
    $session_creation_result = exec($session_creation, $output, $status);
    if($status == 0) {
        drush_print($session_creation_result);
    } else {
        drush_die('Session creation failed.', 1);
    }
}

/**
 *
 * run all tests - this is the main part of this drush command
 * @param bool $log_in_must
 * @param $jasmine_integration_module_path
 * @param $manifest_object
 * @param $drupal_url
 * @param $phantom_session_file
 */

function drupal_jasmine_integration_run_all_tests($log_in_must = FALSE, $jasmine_integration_module_path, $manifest_object, $drupal_url,$phantom_session_file ) {
    foreach ($manifest_object as $test => $test_details) {
        $base_url = $drupal_url.'/?q=';
        $url = $base_url.$test_details->url;
        if(($log_in_must == TRUE && $test_details->registered == '1') || ($log_in_must == FALSE && $test_details->registered == '0')) {
            drush_print("Going to run tests for $url");
            drush_print(shell_exec("phantomjs --cookies-file=$phantom_session_file $jasmine_integration_module_path/js/run-jasmine.js $url"));
        }
    }
}
