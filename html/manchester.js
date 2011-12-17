/*
Java-based manchester encoding script
by Mike Szczys

This script is passed a message from a webpage. It then
encodes that message by flashing the background color
black or white based on manchester encoding.

The message is preceded by two bytes, one for the 
receiving device to use for sync, the next as a preamble.

http://jumptuck.com
*/

var T_TIME = 100; 	//Set T (half a bit frame) in milliseconds here

var message="Hi";
var bit_tracker = (1<<7);
var curByte = 0x00;
var byte_tracker = -2;	//Send one sink byte, then one preamble byte before message

var utility = [0xFF,0x0F];	//Sink byte and preamble byte

function man_zero(step)
{
	//high-to-low
	if(step) {
		//This is the mid-point of the bit frame
		document.bgColor='white';
		setTimeout("man_zero(0)",T_TIME);
	}
	else {
		//This is the start of the bit frame
		document.bgColor='black';
		setTimeout("send_byte()",T_TIME);
	}
}

function man_one(step)
{
	//low-to-high
	if(step) {
		//This is the mid-point of the bit frame
		document.bgColor='black';
		setTimeout("man_one(0)",T_TIME);
	}
	else {
		//This is the start of the bit frame
		document.bgColor='white';
		setTimeout("send_byte()",T_TIME);
	}
}

function send_message()
{
	if (byte_tracker < 0) {
		curByte = utility[byte_tracker+2];
		bit_tracker = (1<<7);
		send_byte();
	}
	else if (byte_tracker < message.length) {
		curByte = message.charCodeAt(byte_tracker);
		bit_tracker = (1<<7);
		send_byte();
	}
	else { document.bgColor = 'white'; } //Message has been sent. Reset background color
}

function send_byte()
{
	if (bit_tracker) {
		if (curByte & (bit_tracker)){ 
			bit_tracker = (bit_tracker>>1);
			document.write("1");
			man_one(1); }
		else { 
			bit_tracker = (bit_tracker>>1);
			document.write("0");
			man_zero(1); }
	}
	else { 
		byte_tracker = byte_tracker + 1; 
		send_message();
	}
}

function messageInput(form) {
	message=form.inputbox.value;
	//setup all default values:
	bit_tracker = (1<<7);
	curByte = 0x00;
	byte_tracker = -2;	//Send one sink byte, then one preamble byte before message
	send_message()
}