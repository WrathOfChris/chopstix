<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<!-- $Gateweaver: order.php,v 1.2 2007/10/11 23:21:20 cmaxwell Exp $ -->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<title>Chopstix: Order</title>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
	<meta http-equiv="imagetoolbar" content="no" />
	<link rel="stylesheet" type="text/css" href="/style.css" />
	<link rel="shortcut icon" href="/favicon.ico" />
</head>
<body>
<div id="main">

<?
	if (strncmp($_POST["cmd"], "Print", 5) != 0) {
?>
<?php include("../../header.php"); ?>
<div class="outer">
<div class="inner">
<div class="float-wrap">
	<div id="content">
		<div class="contentWrap">
<?	} ?>

<? include("order_form.php"); ?>

<?	if (strncmp($_POST["cmd"], "Print", 5) != 0) { ?>
		</div>
	</div>

	<?php include("menu.php"); ?>
	<div class="clear"></div>

</div><!-- float-wrap -->

<?php include("../../menu.php"); ?>
<div class="clear"></div>

</div><!-- inner -->
</div><!-- outer -->
<?	} ?>
</div><!-- main -->

<?  if (strncmp($_POST["cmd"], "Print", 5) != 0) { ?>
<?php include("../../footer.php"); ?>
<?	} ?>

</body>
</html>
