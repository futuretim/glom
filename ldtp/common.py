#!/usr/bin/python

import threading
import ldtp
import ldtputils

main_window = '*Glom*'
initial_dialog = 'WelcometoGlom'

central_info = []

# Radio button texts in the database creation dialog
backend_create_db_button_texts = {
	'PostgresCentral': [
		'CreatedatabaseonanexternalPostgreSQLdatabaseserver,tobespecifiedinthenextstep.',
		'Createdatabaseonanexternaldatabaseserver,tobespecifiedinthenextstep.'
	], 'PostgresSelf': [
		'CreatePostgreSQLdatabaseinitsownfolder,tobehostedbythiscomputer.',
		'Createdatabaseinitsownfolder,tobehostedbythiscomputer.'
	], 'SQLite': [
		'CreateSQLitedatabaseinitsownfolder,tobehostedbythiscomputer.',
		'Createdatabaseinitsownfolder,tobehostedbythiscomputer,usingSQLite.'
	]
}

def launch_glom():
	# Start glom:
	ldtp.launchapp('glom')

	# Wait for the initial dialog to appear. The argument matches on the
	# Window title, so make sure to set a window title in Glade or in the
	# code for each window.
	# Wildcard (* and ?) can be used.
	if ldtp.waittillguiexist(main_window) == 0:
		raise ldtp.LdtpExecutionError('Glom main window does not show up')
	if ldtp.waittillguiexist(initial_dialog) == 0:
		raise ldtp.LdtpExecutionError('Glom initial dialog does not show up')

def exit_glom():
	ldtp.selectmenuitem(main_window, 'mnuFile;mnuClose')

	if ldtp.waittillguinotexist('Glom-SmallBusinessExample') == 0:
		raise ldtp.LdtpExecutionError('The Glom Window does not disappear after closing the application')

# Reads hostname, username and password to use for access to a
# centrally hosted database server
def read_central_info():
	global central_info
	if len(central_info) > 0:
		return central_info

	info = ldtputils.LdtpDataFileParser('central-info.xml')
	ret = [info.gettagvalue('hostname'), info.gettagvalue('username'), info.gettagvalue('password')]

	if len(ret[0]) == 0 or len(ret[1]) == 0 or len(ret[2]) == 0:
		raise ldtp.LdtpExecutionError('Connection details for centrally hosted database not provided. See the README file for how to provide the details.')

	central_info = [ret[0][0], ret[1][0], ret[2][0]]
	return central_info

def enter_connection_credentials(backend_name):
	if backend_name == 'PostgresCentral':
		if ldtp.waittillguiexist('Connection Details') == 0:
			raise ldtp.LdtpExecutionError('Connection details dialog does not show up')

		(hostname, username, password) = read_central_info()

		# Set connection details
		ldtp.settextvalue('Connection Details', 'txtHost', hostname)
		ldtp.settextvalue('Connection Details', 'txtUser', username)
		ldtp.settextvalue('Connection Details', 'txtPassword', password)

		# Acknowledge the dialog
		ldtp.click('Connection Details', 'btnConnect')

		# Make sure it's gone
		if ldtp.waittillguinotexist('Connection Details') == 0:
			raise ldtp.LdtpExecutionError('Connection details dialog does not disappear')

# Selects one of the backends in button_texts in the database creation dialog
def select_backend(backend_name):
	try:
		button_texts = backend_create_db_button_texts[backend_name]
	except KeyError:
		raise ldtp.LdtpExecutionError('Backend "' + backend + '" does not exist')

	for text in button_texts:
		if ldtp.objectexist('Creating From Example File', 'rbtn' + text):
			ldtp.click('Creating From Example File', 'rbtn' + text)
			break
	else:
		raise ldtp.LdtpExecutionError('Backend "' + backend + '" not supported')

def wait_for_database_open():
	# Wait for the list view to pop up in the main Glom window:
	# Note that the Window title of the Glom Window changes when the file
	# has loaded. If we use wildcards for the Window title (*Glom*) here,
	# then objectexist does not find the notebook_data
	# (ptlListOrDetailsView) widget, even when it has actually appeared.
	# TODO: Maybe we can use setcontext(), to avoid this.
	# TODO: Or maybe this has been fixed in LDTP in the meanwhile,
	# see bug #583021.
	while not ldtp.guiexist('Glom-SmallBusinessExample') or not ldtp.objectexist('Glom-SmallBusinessExample', 'ptlListOrDetailsView'):
		# onwindowcreate calls the callback in a new thread, which
		# does not really help us since we don't have a mainloop the
		# callback thread could notify, so we would need to have to
		# poll an event anyway. Instead, we can simply poll directly
		# the existance of an error dialog.
		# Plus, there seems to be a bug in LDTP when running a test
		# sequence of multiple tests using onwindowcreate:
		# http://bugzilla.gnome.org/show_bug.cgi?id=586291.
		if ldtp.guiexist('Warning') or ldtp.guiexist('Error'):
			# TODO: Read error message from error dialog
			raise ldtp.LdtpExecutionError('Failed to create new database')

		# Wait a bit and then try again:
		ldtp.wait()

def check_small_business_integrity():
	if(ldtp.getrowcount(main_window, 'tblTableContent') != 9):
		raise ldtp.LdtpExecutionError("Newly created database does not contain all 8 rows"); # Note there is one placeholder row

	ldtp.selecttab(main_window, 'ptlListOrDetailsView', 'Details')

	# TODO: Check that there is an image present.
	# Accerciser shows the Image Size for the widget which we may use to
	# check this. However, LDTP does not implement this yet.
