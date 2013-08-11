<!-- $Gateweaver: order_form.php,v 1.3 2007/10/15 00:45:43 cmaxwell Exp $ -->
<form method="post" action="order.php">
<?
	function P($post) {
		if (strlen($_POST[$post]) == 0)
			return "&nbsp;";
		else
			return $_POST[$post];
	}

	$row = 1;
	function ROW() {
		global $row;
		$row = !$row;
		return $row;
	}

	define("PAGE_INSTRUCT", 0);
	define("PAGE_CUSTOMER", 1);
	define("PAGE_LICENCE", 2);
	define("PAGE_ORDER", 3);
	define("PAGE_MAX", 4);
	define("PRICE_CHOPSTIX", 200000);
	define("PRICE_RULE", 10000);
	define("PRICE_MENU", 500);
	define("PRICE_YEAR", 50000);
	define("PRICE_HOUR", 12500);

	$preview = 0;

	$page = $_POST["page"];
	if ($_POST["cmd"] == "Back")
		$page = max(0, $_POST["page"] - 1);
	else if ($_POST["cmd"] == "Next")
		$page = min($_POST["page"] + 1, PAGE_MAX);
	else if ($_POST["cmd"] == "Return" && $page >= PAGE_MAX)
		$page = PAGE_MAX - 1;
	else if ($_POST["cmd"] == "Preview" || strncmp($_POST["cmd"], "Print", 5) == 0)
		$preview = 1;

	if (!isset($_POST["o_qty_cstx"]))
		$_POST["o_qty_cstx"] = 1;
	if (!isset($_POST["o_qty_rule"]))
		$_POST["o_qty_rule"] = 0;
	if (!isset($_POST["o_qty_menu"]))
		$_POST["o_qty_menu"] = 0;
	if (!isset($_POST["o_qty_year"]))
		$_POST["o_qty_year"] = 0;
	if (!isset($_POST["o_qty_hour"]))
		$_POST["o_qty_hour"] = 0;

	printf("<input type=\"hidden\" name=\"page\" value=\"%s\" />", $page);

	if ($page >= PAGE_MAX)
		$preview = 1;
	if ($preview)
		$page = PAGE_MAX;

	if ($page == PAGE_INSTRUCT) {
		printf("<h2>Ordering Instructions (Page %d of %d)</h2>",
				$page + 1, PAGE_MAX);
		print("<p>Manorsoft does not currently accept online orders.  Please
				fill out the form on the next few pages which will create a
				printable sales order that can be mailed to:</p>
				<ul><b>Manorsoft</b><br />
				No longer active<br />
				Nowhere, Ontario<br />
				H0H 0H0</ul>");
	}

	if ($page == PAGE_CUSTOMER) {
		printf("<h2>Customer Information (Page %d of %d)</h2>",
				$page + 1, PAGE_MAX);
		print("Please enter the <i>purchasor's</i> billing name and address
				below.  This is the name of the corporation or sole proprietor
				who is purchasing the software.");

		print("<table>");

		print("<tr><td colspan=\"4\"><b>Company</b></td></tr>");
		printf("<tr><td colspan=\"4\"><input type=\"text\" name=\"c_company\" value=\"%s\" size=\"61\"/></td>",
				$_POST["c_company"]);

		print("<tr><td colspan=\"4\"><b>Contact</b></td></tr>");
		printf("<tr><td colspan=\"4\"><input type=\"text\" name=\"c_contact\" value=\"%s\" size=\"61\"/></td>",
				$_POST["c_contact"]);

		print("<tr><td colspan=\"4\"><b>Street Address</b></td></tr>");
		printf("<tr><td colspan=\"4\"><input type=\"text\" name=\"c_address\" value=\"%s\" size=\"61\"/></td>",
				$_POST["c_address"]);

		print("<tr><td colspan=\"2\"><b>City</b></td>");
		print("<td><b>Province</b></td>");
		print("<td><b>Postal Code</b></td></tr>");
		printf("<tr><td colspan=\"2\"><input type=\"text\" name=\"c_city\" value=\"%s\" size=\"30\"/></td>",
				$_POST["c_city"]);
		printf("<td><input type=\"text\" name=\"c_province\" value=\"%s\" size=\"14\"/></td>",
				$_POST["c_province"]);
		printf("<td><input type=\"text\" name=\"c_postal\" value=\"%s\" size=\"14\"/></td></tr>",
				$_POST["c_postal"]);
		print("<tr><td colspan=\"2\"><b>Phone</b></td>");
		print("<td colspan=\"2\"><b>Fax</b></td></tr>");
		printf("<td colspan=\"2\"><input type=\"text\" name=\"c_phone\" value=\"%s\" size=\"30\"/></td>",
				$_POST["c_phone"]);
		printf("<td colspan=\"2\"><input type=\"text\" name=\"c_fax\" value=\"%s\" size=\"30\"/></td></tr>",
				$_POST["c_fax"]);
		print("<tr><td colspan=\"4\"><b>Email Address</b></td></tr>");
		printf("<tr><td colspan=\"4\"><input type=\"text\" name=\"c_email\" value=\"%s\" size=\"61\"/></td>",
				$_POST["c_email"]);

		print("</table>");
	} else {
		printf("<input type=\"hidden\" name=\"c_company\" value=\"%s\" />", $_POST["c_company"]);
		printf("<input type=\"hidden\" name=\"c_contact\" value=\"%s\" />", $_POST["c_contact"]);
		printf("<input type=\"hidden\" name=\"c_address\" value=\"%s\" />", $_POST["c_address"]);
		printf("<input type=\"hidden\" name=\"c_city\" value=\"%s\" />", $_POST["c_city"]);
		printf("<input type=\"hidden\" name=\"c_province\" value=\"%s\" />", $_POST["c_province"]);
		printf("<input type=\"hidden\" name=\"c_postal\" value=\"%s\" />", $_POST["c_postal"]);
		printf("<input type=\"hidden\" name=\"c_phone\" value=\"%s\" />", $_POST["c_phone"]);
		printf("<input type=\"hidden\" name=\"c_fax\" value=\"%s\" />", $_POST["c_fax"]);
		printf("<input type=\"hidden\" name=\"c_email\" value=\"%s\" />", $_POST["c_email"]);
	}

	if ($page == PAGE_LICENCE) {
		printf("<h2>Licence Information (Page %d of %d)</h2>",
				$page + 1, PAGE_MAX);
		print("Please enter the <i>licencee's</i> operating name and address
				below.  This is the name and address that will appear on all
				order receipts.");
		print("<table>");

		print("<tr><td colspan=\"2\"><b>Operating Name</b></td>");
		print("<td colspan=\"2\"><b>Operating Phone</b></td></tr>");
		printf("<tr><td colspan=\"2\"><input type=\"text\" name=\"l_company\" value=\"%s\" size=\"30\"/></td>",
				$_POST["l_company"]);
		printf("<td colspan=\"2\"><input type=\"text\" name=\"l_phone\" value=\"%s\" size=\"30\"/></td></tr>",
				$_POST["l_phone"]);

		print("<tr><td colspan=\"4\"><b>Street Address</b></td></tr>");
		printf("<tr><td colspan=\"4\"><input type=\"text\" name=\"l_address\" value=\"%s\" size=\"61\"/></td>",
				$_POST["l_address"]);

		print("<tr><td colspan=\"2\"><b>City</b></td>");
		print("<td><b>Province</b></td>");
		print("<td><b>Postal Code</b></td></tr>");
		printf("<tr><td colspan=\"2\"><input type=\"text\" name=\"l_city\" value=\"%s\" size=\"30\"/></td>",
				$_POST["l_city"]);
		printf("<td><input type=\"text\" name=\"l_province\" value=\"%s\" size=\"14\"/></td>",
				$_POST["l_province"]);
		printf("<td><input type=\"text\" name=\"l_postal\" value=\"%s\" size=\"14\"/></td></tr>",
				$_POST["l_postal"]);

		print("</table>");
	} else {
		printf("<input type=\"hidden\" name=\"l_company\" value=\"%s\" />", $_POST["l_company"]);
		printf("<input type=\"hidden\" name=\"l_address\" value=\"%s\" />", $_POST["l_address"]);
		printf("<input type=\"hidden\" name=\"l_city\" value=\"%s\" />", $_POST["l_city"]);
		printf("<input type=\"hidden\" name=\"l_province\" value=\"%s\" />", $_POST["l_province"]);
		printf("<input type=\"hidden\" name=\"l_postal\" value=\"%s\" />", $_POST["l_postal"]);
		printf("<input type=\"hidden\" name=\"l_phone\" value=\"%s\" />", $_POST["l_phone"]);
		printf("<input type=\"hidden\" name=\"l_fax\" value=\"%s\" />", $_POST["l_fax"]);
	}

	if ($page == PAGE_ORDER) {
		printf("<h2>Order Information (Page %d of %d)</h2>", $page + 1, PAGE_MAX);
		print("<div class=\"print_totals\">");
		print("<table>");

		print("<tr><th>Quantity</th><th>Description</th><th>Price</th></tr>");

		printf("<tr valign=\"top\"><td align=\"center\"><input type=\"text\"
				name=\"o_qty_cstx\" value=\"%s\" size=\"5\"/></td>",
				$_POST["o_qty_cstx"]);
		print("<td><b>Chopstix Software</b><ul>
				<li>Site Licence - Lifetime</li>
				<li>Upgrade & Support Contract - One (1) Year</li>
				<li>Custom Business Rules - Five (5) Rules</li>
				<li>Menu Item Data Entry - One Hundred (100) Items</li>
				</ul></td><td align=\"right\">$2000.00</td></tr>");

		printf("<tr valign=\"top\"><td align=\"center\"><input type=\"text\"
				name=\"o_qty_rule\" value=\"%s\" size=\"5\"/></td>",
				$_POST["o_qty_rule"]);
		print("<td><b>Chopstix Additional Custom Business Rule</b></td>
				<td align=\"right\">$100.00</td></tr>");

		printf("<tr valign=\"top\"><td align=\"center\"><input type=\"text\"
				name=\"o_qty_menu\" value=\"%s\" size=\"5\"/></td>",
				$_POST["o_qty_menu"]);
		print("<td><b>Chopstix Additional Menu Item Data Entry</b></td>
				<td align=\"right\">$5.00</td></tr>");

		printf("<tr valign=\"top\"><td align=\"center\"><input type=\"text\"
				name=\"o_qty_year\" value=\"%s\" size=\"5\"/></td>",
				$_POST["o_qty_year"]);
		print("<td><b>Chopstix One (1) Year Upgrade & Support Contract</b></td>
				<td align=\"right\">$500.00</td></tr>");

		printf("<tr valign=\"top\"><td align=\"center\"><input type=\"text\"
				name=\"o_qty_hour\" value=\"%s\" size=\"5\"/></td>",
				$_POST["o_qty_hour"]);
		print("<td><b>Onsite Consulting Hour</b></td>
				<td align=\"right\">$125.00</td></tr>");

		print("</table>");
		print("</div>");
	} else {
		printf("<input type=\"hidden\" name=\"o_qty_cstx\" value=\"%s\" />",
				$_POST["o_qty_cstx"]);
		printf("<input type=\"hidden\" name=\"o_qty_rule\" value=\"%s\" />",
				$_POST["o_qty_rule"]);
		printf("<input type=\"hidden\" name=\"o_qty_menu\" value=\"%s\" />",
				$_POST["o_qty_menu"]);
		printf("<input type=\"hidden\" name=\"o_qty_year\" value=\"%s\" />",
				$_POST["o_qty_year"]);
		printf("<input type=\"hidden\" name=\"o_qty_hour\" value=\"%s\" />",
				$_POST["o_qty_hour"]);
	}

	if ($preview) {
		if (strncmp($_POST["cmd"], "Print", 5) != 0) {
			print("<font color=\"red\"><i>
					Please verify all information is correct before proceeding.
					To return to the order form, press the <b>Return</b> button
					located at the bottom of the page.  No information will be
					sent to Manorsoft unless you press the <b>Print & Send</b>
					button at the bottom of the page.  </i></font>");
		}

		/* Header */
		print("<table class=\"print\">");
		print("<tr><td><img src=\"/images/ChopstixLogo.gif\"></td>");
		print("<td align=\"right\"><h1>Sales Order</h1></td></tr>");
		print("<tr><td><h4>Manorsoft</h4>");
		print("No longer active<br />Nowhere, Ontario<br />H0H 0H0<br />(905) 000-0000</td>");
		printf("<td align=\"right\" valign=\"top\">Date: %s</td></tr>", date("F j, Y"));
		print("</table>");

		/* Customer */
		print("<br />");
		print("<div class=\"print_border\">");
		print("<h2>Customer Information</h2>");
		print("<table>");
		print("<tr><th colspan=\"4\">Company</th></tr>");
		printf("<tr><td colspan=\"4\">%s</td></tr>", P("c_company"));
		print("<tr><th colspan=\"4\">Contact</th></tr>");
		printf("<tr><td colspan=\"4\">%s</td></tr>", P("c_contact"));
		print("<tr><th colspan=\"4\">Street Address</th></tr>");
		printf("<tr><td colspan=\"4\">%s</td></tr>", P("c_address"));
		print("<tr><th colspan=\"2\">City</th><th>Province</th><th>Postal Code</th></tr>");
		printf("<tr><td colspan=\"2\">%s</td><td>%s</td><td>%s</td></tr>",
				P("c_city"), P("c_province"), P("c_postal"));
		print("<tr><th colspan=\"2\">Phone</th><th colspan=\"2\">Fax</th></tr>");
		printf("<tr><td colspan=\"2\">%s</td><td colspan=\"2\">%s</td></tr>",
				P("c_phone"), P("c_fax"));
		print("<tr><th colspan=\"4\">Email Address</th></tr>");
		printf("<tr><td colspan=\"4\">%s</td></tr>", P("c_email"));
		print("</table>");
		print("</div>");

		/* Licence */
		print("<br />");
		print("<div class=\"print_border\">");
		print("<h2>Licence Information</h2>");
		print("<table>");
		print("<tr><th colspan=\"2\">Operating Name</th><th colspan=\"2\">Operating Phone</th></tr>");
		printf("<tr><td colspan=\"2\">%s</td><td colspan=\"2\">%s</td></tr>",
				P("l_company"), P("l_phone"));
		print("<tr><th colspan=\"4\">Street Address</th></tr>");
		printf("<tr><td colspan=\"4\">%s</td></tr>", P("l_address"));
		print("<tr><th colspan=\"2\">City</th><th>Province</th><th>Postal Code</th></tr>");
		printf("<tr><td colspan=\"2\">%s</td><td>%s</td><td>%s</td></tr>",
				P("l_city"), P("l_province"), P("l_postal"));
		print("</table>");
		print("</div>");

		/* Order */
		print("<br />");
		print("<div class=\"print_totals\">");
		print("<h2>Order Information</h2>");
		print("<table><font size=\"-1\"");
		print("<tr><th>Qty</th><th>Code</th><th>Description</th><th>Unit Price</th><th>Line Total</th></tr>");

		if ($_POST["o_qty_cstx"] > 0) {
			printf("<tr class=\"row%d\">", ROW());
			printf("<td align=\"center\">%d</td>", $_POST["o_qty_cstx"]);
			printf("<td align=\"center\">CSTX-1Y-5R-100M</td>");
			printf("<td>Chopstix Site Licence - Lifetime</td>");
			printf("<td align=\"right\">$%d.%02d</td>",
					PRICE_CHOPSTIX / 100, PRICE_CHOPSTIX % 100);
			printf("<td align=\"right\">$%d.%02d</td>",
					$_POST["o_qty_cstx"] * PRICE_CHOPSTIX / 100,
					$_POST["o_qty_cstx"] * PRICE_CHOPSTIX % 100);
			print("</tr>");
		}

		if ($_POST["o_qty_rule"] > 0) {
			printf("<tr class=\"row%d\">", ROW());
			printf("<td align=\"center\">%d</td>", $_POST["o_qty_rule"]);
			printf("<td align=\"center\">CSTX-1R</td>");
			printf("<td>Chopstix Additional Custom Business Rule</td>");
			printf("<td align=\"right\">$%d.%02d</td>",
					PRICE_RULE / 100, PRICE_RULE % 100);
			printf("<td align=\"right\">$%d.%02d</td>",
					$_POST["o_qty_rule"] * PRICE_RULE / 100,
					$_POST["o_qty_rule"] * PRICE_RULE % 100);
			print("</tr>");
		}

		if ($_POST["o_qty_menu"] > 0) {
			printf("<tr class=\"row%d\">", ROW());
			printf("<td align=\"center\">%d</td>", $_POST["o_qty_menu"]);
			printf("<td align=\"center\">CSTX-1M</td>");
			printf("<td>Chopstix Additional Menu Item Data Entry</td>");
			printf("<td align=\"right\">$%d.%02d</td>",
					PRICE_MENU / 100, PRICE_MENU % 100);
			printf("<td align=\"right\">$%d.%02d</td>",
					$_POST["o_qty_menu"] * PRICE_MENU / 100,
					$_POST["o_qty_menu"] * PRICE_MENU % 100);
			print("</tr>");
		}

		if ($_POST["o_qty_year"] > 0) {
			printf("<tr class=\"row%d\">", ROW());
			printf("<td align=\"center\">%d</td>", $_POST["o_qty_year"]);
			printf("<td align=\"center\">CSTX-1Y</td>");
			printf("<td>Chopstix Upgrade & Support Contract - One (1) Year</td>");
			printf("<td align=\"right\">$%d.%02d</td>",
					PRICE_YEAR / 100, PRICE_YEAR % 100);
			printf("<td align=\"right\">$%d.%02d</td>",
					$_POST["o_qty_year"] * PRICE_YEAR / 100,
					$_POST["o_qty_year"] * PRICE_YEAR % 100);
			print("</tr>");
		}

		if ($_POST["o_qty_hour"] > 0) {
			printf("<tr class=\"row%d\">", ROW());
			printf("<td align=\"center\">%d</td>", $_POST["o_qty_hour"]);
			printf("<td align=\"center\">CM-HR</td>");
			printf("<td>Onsite Consulting - One (1) Hour</td>");
			printf("<td align=\"right\">$%d.%02d</td>",
					PRICE_HOUR / 100, PRICE_HOUR % 100);
			printf("<td align=\"right\">$%d.%02d</td>",
					$_POST["o_qty_hour"] * PRICE_HOUR / 100,
					$_POST["o_qty_hour"] * PRICE_HOUR % 100);
			print("</tr>");
		}

		$subtotal = $_POST["o_qty_cstx"] * PRICE_CHOPSTIX +
			$_POST["o_qty_rule"] * PRICE_RULE +
			$_POST["o_qty_menu"] * PRICE_MENU +
			$_POST["o_qty_year"] * PRICE_YEAR +
			$_POST["o_qty_hour"] * PRICE_HOUR;
		$tax1 = $subtotal * 0.08;
		$tax2 = $subtotal * 0.06;
		$total = $subtotal + $tax1 + $tax2;

		print("<tr><td colspan=\"3\" rowspan=\"4\" valign=\"top\">");
		print("<font size=\"-2\">Payment Details<ul><font size=\"-2\">");
		printf("<li>Ontario residents add 8%% PST ($%d.%02d)</li>",
				$tax1 / 100, $tax1 % 100);
		printf("<li>Manorsoft is currently a <i>Small Supplier</i> and does not charge 6%% GST ($%d.%02d)</li>",
				$tax2 / 100, $tax2 % 100);
		print("<li>Please make cheques payable to Chris Maxwell</li>");
		print("</font></ul></font></td>");
		$crow = ROW();
		printf("<td class=\"row%d\" align=\"right\">Subtotal</td>
				<td class=\"row%d\" align=\"right\">$%d.%02d</td></tr>",
				$crow, $crow, $subtotal / 100, $subtotal % 100);
		printf("<tr class=\"row%d\"><td align=\"right\">PST</td>
				<td align=\"right\">&nbsp;<br />&nbsp;</td></tr>", ROW());
		printf("<tr class=\"row%d\"><td align=\"right\">GST</td>
				<td align=\"right\">&nbsp;<br />&nbsp;</td></tr>", ROW());
		printf("<tr class=\"row%d\"><td align=\"right\">Total</td>
				<td align=\"right\">&nbsp;<br />&nbsp;</td></tr>", ROW());

		print("</table>");
		print("</div>");

		/* Authorized */
		print("<br />");
		print("<br />");
		print("<br />");
		print("<br />");
		print("<div class=\"bordertop\"><font size=\"-2\">");
		print("<table width=\"80%\">");
		print("<tr><td>Authorized By</td><td>Authorized Signature</td><td>Date</td></tr>");
		print("</table></font>");
		print("</div>");

		if (strncmp($_POST["cmd"], "Print", 5) != 0) {
			/* Instructions */
			print("<br />");
			print("<br />");
			print("<font color=\"red\"><i>
					The <b>Print & Send</b> button will notify Manorsoft of
					your order and will prepare the order form for printing.
					Please mail your order to the address listed at the top of
					the order.
					If you prefer to not to notify Manorsoft, use the
					<b>Print</b> which will only prepare the order form for
					printing.</i></font>");
		}
	}

	if (strncmp($_POST["cmd"], "Print", 5) != 0) {
		print("<br />");
		print("<input type=\"submit\" name=\"cmd\" value=\"Back\" />\n");
		if ($preview) {
			print("<input type=\"submit\" name=\"cmd\" value=\"Return\" />\n");
			print("<input type=\"submit\" name=\"cmd\" value=\"Print\" />\n");
			print("<input type=\"submit\" name=\"cmd\" value=\"Print & Send\" />\n");
		} else {
			print("<input type=\"submit\" name=\"cmd\" value=\"Preview\" />\n");
			print("<input type=\"submit\" name=\"cmd\" value=\"Next\" />\n");
		}
	}

	if ($_POST["cmd"] == "Print & Send") {
		$subject = sprintf("Chopstix Order: %s", $_POST["c_company"]);
		$smsmessage = sprintf(
				"#: %s\n" .
				"C: %s\n" .
				"Q: %dxCSTX, %dxRULE, %dxMENU, %dxYEAR, %dxHOUR\n" .
				"T: $%d.%02d\n" .
				"D: %s\n" .
				"%s",
				$_POST["c_phone"],
				$_POST["c_contact"],
				$_POST["o_qty_cstx"],
				$_POST["o_qty_rule"],
				$_POST["o_qty_menu"],
				$_POST["o_qty_year"],
				$_POST["o_qty_hour"],
				$subtotal / 100, $subtotal % 100,
				date("m/d/Y Y H:m"),
				$_POST["c_copmany"]);
		$message = sprintf(
				"Chopstix Order\n" .
				"==============\n" .
				"Date: %s\n" .
				"\n" .
				"Billing Information\n" .
				"-------------------\n" .
				"%s\n" .
				"attn: %s\n" .
				"%s\n" .
				"%s, %s, %s\n" .
				"Phone: %s\n" .
				"Fax: %s\n" .
				"Email: %s\n" .
				"\n" .
				"Licence Information\n" .
				"-------------------\n" .
				"%s\n" .
				"%s\n" .
				"%s, %s, %s\n" .
				"Phone: %s\n" .
				"\n" .
				"Order Information\n" .
				"-----------------\n" .
				"CSTX: %d\n" .
				"RULE: %d\n" .
				"MENU: %d\n" .
				"YEAR: %d\n" .
				"HOUR: %d\n" .
				"TOTAL: $%d.%02d\n",
				date("F j, Y H:m"),
				$_POST["c_company"],
				$_POST["c_contact"],
				$_POST["c_address"],
				$_POST["c_city"], $_POST["c_province"], $_POST["c_postal"],
				$_POST["c_phone"],
				$_POST["c_fax"],
				$_POST["c_email"],
				$_POST["l_company"],
				$_POST["l_address"],
				$_POST["l_city"], $_POST["l_province"], $_POST["l_postal"],
				$_POST["l_phone"],
				$_POST["o_qty_cstx"],
				$_POST["o_qty_rule"],
				$_POST["o_qty_menu"],
				$_POST["o_qty_year"],
				$_POST["o_qty_hour"],
				$subtotal / 100, $subtotal % 100);
		mail("sales@manorsoft.ca", $subject, $message);
	}
?>
</form>
