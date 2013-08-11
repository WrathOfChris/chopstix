<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<!-- $Gateweaver: index.php,v 1.2 2007/10/15 16:56:17 cmaxwell Exp $ -->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<title>Products</title>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
	<meta http-equiv="imagetoolbar" content="no" />
	<link rel="stylesheet" type="text/css" href="/style.css" />
	<link rel="shortcut icon" href="/favicon.ico" />
</head>
<body>
<div id="main">
<?php include("../../../header.php"); ?>
<div class="outer">
<div class="inner">
<div class="float-wrap">
	<div id="content">
		<div class="contentWrap">

<p><h2>Chopstix Screenshots</h2>
<ul>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixDesktopFull">
		Order Entry Screen</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixEmpty">
		Order Entry Screen (empty)</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixComplete">
		Order Entry (completed)</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixCompleteLabelled">
		Order Entry (labelled)</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixDesktopMenu">
		Desktop Menu</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixReportOrderList">
		Report: Orders List</a></li>
	<li><a href="/products/chopstix/screenshots/?screen=ChopstixReceiptSimple">
		Receipt</a></li>
</ul>
</p>

<?php
	if ($_REQUEST["screen"])
		print "<p><img src=\"" . $_REQUEST["screen"] . ".png\"></p>";
?>

		</div>
	</div>

	<?php include("../menu.php"); ?>
	<div class="clear"></div>

</div><!-- float-wrap -->

<?php include("../../../menu.php"); ?>
<div class="clear"></div>

</div><!-- inner -->
</div><!-- outer -->
</div><!-- main -->

<?php include("../../../footer.php"); ?>

</body>
</html>
