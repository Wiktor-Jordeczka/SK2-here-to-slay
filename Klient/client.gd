extends Node

signal connected
signal data
signal disconnected
signal error

var _status: int = 0
var _stream: StreamPeerTCP = StreamPeerTCP.new()

func _ready() -> void:
	_status = _stream.get_status()

func _process(delta: float) -> void:
	_stream.poll()
	var new_status: int = _stream.get_status()
	if new_status != _status:
		_status = new_status
		match _status:
			_stream.STATUS_NONE:
				print("Disconnected from host.")
				emit_signal("disconnected")
			_stream.STATUS_CONNECTING:
				print("Connecting to host.")
			_stream.STATUS_CONNECTED:
				print("Connected to host.")
				#_stream.put_data("1:klasa:getVal".to_ascii_buffer())
				emit_signal("connected")
			_stream.STATUS_ERROR:
				print("Error with socket stream.")
				emit_signal("error")

	if _status == _stream.STATUS_CONNECTED:
		var available_bytes: int = _stream.get_available_bytes()
		if available_bytes > 0:
			print("available bytes: ", available_bytes)
			var data = _stream.get_partial_data(256)
			if data[0] != OK:
				print("Error getting data from stream: ", data[0])
				emit_signal("error")
			else:
				print("Recived data")
				emit_signal("data", data[1].get_string_from_utf8())
				#print(data[1].get_string_from_utf8())

func connect_to_host(host: String, port: int) -> void:
	print("Connecting to %s:%d" % [host, port])
	_status = _stream.STATUS_NONE
	#print(_stream.connect_to_host(host, port))
	if _stream.connect_to_host(host, port) != OK:
		print("Error connecting to host.")
		emit_signal("error")
	
func send(data: String) -> bool:
	#_stream.put_data("1:klasa:getVal".to_ascii_buffer())
	if _status != _stream.STATUS_CONNECTED:
		print("Error: Stream is not currently connected.")
		return false
	var error: int = _stream.put_data(data.to_ascii_buffer())
	if error != OK:
		print("Error writing to stream: ", error)
		return false
	return true
