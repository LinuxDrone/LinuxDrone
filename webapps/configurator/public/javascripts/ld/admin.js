/**
 * Created by achernikov on 15.05.14.
 */
$(document).ready(function(){
    var $aW = $('#js-admin-panel'),
        $tB = $('.js-admin-toggle');
    $tB.click(function(){
         $aW.toggleClass('expanded');
    })
});