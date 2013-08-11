<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<!-- $Gateweaver: custom.php,v 1.1 2007/10/10 02:35:25 cmaxwell Exp $ -->
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
<head>
	<title>Chopstix: Customization</title>
	<meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
	<meta http-equiv="imagetoolbar" content="no" />
	<link rel="stylesheet" type="text/css" href="/style.css" />
	<link rel="shortcut icon" href="/favicon.ico" />
</head>
<body>
<div id="main">
<?php include("../../header.php"); ?>
<div class="outer">
<div class="inner">
<div class="float-wrap">
	<div id="content">
		<div class="contentWrap">

<p><h2>Custom Business Rules</h2>
Chopstix contains a powerful plugin loader that allows implementation of your
business rules.  <a href="/contact">Contact us</a> for more information.</p>

<p>Some examples:
<ul>
	<li>Add one free order of chicken wings on orders over $30.00 before taxes,
		excluding combos.</li>
	<li>Add one fortune cookie for every $10.00 or part thereof.</li>
	<li>Discount 10% on cash orders greater than $20.00.</li>
	<li>Free delivery on orders over $15.00 before taxes, $3.00 otherwise.</li>
	<li>When order item is "Chicken Balls" or "Deep Fried Shrimp", based on
		style choice add dipping sauce "Sweet &amp; Sour" or "Honey Garlic"</li>
</ul></p>

		</div>
	</div>

	<?php include("menu.php"); ?>
	<div class="clear"></div>

</div><!-- float-wrap -->

<?php include("../../menu.php"); ?>
<div class="clear"></div>

</div><!-- inner -->
</div><!-- outer -->
</div><!-- main -->

<?php include("../../footer.php"); ?>

</body>
</html>
