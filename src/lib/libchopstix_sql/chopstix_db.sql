-- $Gateweaver: chopstix_db.sql,v 1.11 2007/09/29 03:21:57 cmaxwell Exp $
--
-- Chopstix Schema
--

--	ChopstixCustomer ::= SEQUENCE {
--		phone[0]		ChopstixPhone,
--		name[1]			ChopstixName,
--		addr[2]			ChopstixAddress,
--		isect[3]		ChopstixIntersect,
--		special[4]		ChopstixSpecial OPTIONAL,
--		credit[5]		ChopstixCredits OPTIONAL
--	}
CREATE TABLE "customers" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"Name" TEXT NOT NULL,
	"Phone" TEXT UNIQUE,
	"PhoneExt" TEXT,
	"Address" TEXT NOT NULL,
	"Apt" TEXT,
	"Entry" TEXT,
	"Intersection" TEXT,
	"Special" TEXT
);

--	ChopstixCredit ::= SEQUENCE {
--		credit[0]		INTEGER,			-- original credit given
--		remain[1]		INTEGER,			-- amount remaining
--		reason[2]		GeneralString		-- why issued
--	}
CREATE TABLE "credits" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"CID" INTEGER,						-- Customer ID
	"Credit" INTEGER,
	"Remain" INTEGER,
	"Reason" TEXT NOT NULL
);
CREATE INDEX credits_CID ON credits ( CID );

--	ChopstixMenuitem ::= SEQUENCE {
--		gen[0]			INTEGER,			-- menu generation number
--		code[1]			ChopstixItemCode,	-- menu code
--		name[2]			GeneralString,		-- item name
--		price[4]		INTEGER,			-- fixedpoint fp
--		styles[5]		ChopstixItemStyles,	-- beef, pork, etc
--		extras[6]		ChopstixItemExtras,	-- condiment extras
--		subitems[7]		ChopstixItemExtras OPTIONAL,	-- for combos, sauces
--		flags[8]		ChopstixFlags
--	}
CREATE TABLE "menuitems" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"Code" TEXT,
	"Generation" INTEGER,
	"Name" TEXT NOT NULL,
	"Price" INTEGER,
	"Deleted" INTEGER,
	UNIQUE ( Code, Generation )
);
CREATE INDEX menuitems_Code ON menuitems ( Code );

--	ChopstixItemStyle ::= SEQUENCE {
--		num[0]			INTEGER,			-- unique ID number, for recalled order
--		name[1]			GeneralString		-- note to print below item
--	}
--
-- To remove a style, set its MID to zero
CREATE TABLE "menustyles" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"MID" INTEGER,						-- Menuitem ID
	"Style" TEXT NOT NULL
);
CREATE INDEX menustyles_MID ON menustyles ( MID );

--	ChopstixItemExtra ::= SEQUENCE {
--		qty[0]			INTEGER,			-- number to include per item
--		code[1]			ChopstixItemCode	-- menu code to reference
--	}
CREATE TABLE "menuextras" (
	"MID" INTEGER,						-- Menuitem ID
	"Qty" INTEGER,
	"Code" TEXT NOT NULL
);
CREATE INDEX menuextras_MID ON menuextras ( MID );

--
-- subitems are initially stored same as menuextras, though later are stored
-- with other information in the order ledger
--
CREATE TABLE "menusubitems" (
	"MID" INTEGER,						-- Menuitem ID
	"Qty" INTEGER,
	"Code" TEXT NOT NULL
);
CREATE INDEX menusubitems_MID ON menusubitems ( MID );

--	ChopstixPayment ::= SEQUENCE {
--		type[0]			CHOPSTIX-PAYMENTTYPE,
--		ccinfo[1]		ChopstixCreditInfo OPTIONAL
--	}
--	ChopstixTotal ::= SEQUENCE {
--		subtotal[0]		INTEGER,
--		discount[1]		INTEGER,			-- override discount
--		delivery[2]		INTEGER,			-- delivery charge if applicable
--		credit[3]		INTEGER,			-- credit applied to order
--		tax1[4]			INTEGER,
--		tax2[5]			INTEGER,
--		total[6]		INTEGER
--	}
--	ChopstixOrder ::= [APPLICATION 1] SEQUENCE {
--		key[0]			ChopstixOrderKey,	-- order number/rev
--		date[1]			ChopstixTime,		-- order posting date
--		customer[2]		ChopstixCustomer,
--		type[3]			CHOPSTIX-ORDERTYPE,
--		payment[4]		ChopstixPayment,
--		items[5]		ChopstixOrderItems,
--		special[6]		ChopstixSpecial,
--		ruleitems[7]	ChopstixOrderItems,
--		total[8]		ChopstixTotal
--	}
CREATE TABLE "orders" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"Date" TEXT DEFAULT CURRENT_TIMESTAMP,
	"CID" INTEGER,						-- Customer ID
	"Type" INTEGER,						-- ORDER_PICKUP, ORDER_DELIVERY,
										-- ORDER_WALKING, ORDER_VOID
	"Special" TEXT,						-- Special instructions, saved per order
	"Paytype" INTEGER,					-- PAYMENT_CASH, PAYMENT_CREDIT
	"Subtotal" INTEGER,
	"Discount" INTEGER,
	"Delivery" INTEGER,
	"Credit" INTEGER,
	"Tax1" INTEGER,
	"Tax2" INTEGER,
	"Total" INTEGER
);
CREATE INDEX orders_CID ON orders ( CID );
CREATE INDEX orders_Date ON orders ( Date );

--
-- Use references to internal OrderID, and MenuID, but copy price in case the
-- price is changed later.
--
CREATE TABLE "orderitems" (
	"ID" INTEGER PRIMARY KEY AUTOINCREMENT,
	"OID" INTEGER,						-- Order ID
	"Line" INTEGER,						-- Order line number
	"Type" INTEGER,						-- ITEM_NORMAL, ITEM_SUBITEM,
										-- ITEM_EXTRA, ITEM_RULE
	"MID" INTEGER,						-- Menuitem ID
	"Qty" INTEGER,						-- Quantity
	"Style" TEXT,						-- Style choice (if applicable)
	"Price" INTEGER,					-- Price, or price delta for subitems
	"Special" TEXT						-- Special instructions, saved per item
);
CREATE INDEX orderitems_OID ON orderitems ( OID );
CREATE INDEX orderitems_MID ON orderitems ( MID );

--	ChopstixCreditNumber ::= GeneralString
--	ChopstixCreditExpiry ::= GeneralString
--	ChopstixCreditInfo ::= SEQUENCE {
--		type[0]			CHOPSTIX-CARDTYPE,
--		number[1]		ChopstixCreditNumber,
--		expiry[2]		ChopstixCreditExpiry
--	}
CREATE TABLE "orderccpayments" (
	"OID" INTEGER,						-- OrderID
	"Number" TEXT,
	"Expiry" TEXT
);
CREATE INDEX orderccpayments_OID ON orderccpayments ( OID );
