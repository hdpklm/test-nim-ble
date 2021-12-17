// https://github.com/don/cordova-plugin-ble-central

window.test = {}
const mBLE1 = {
	connect(address, onconnect, ondisconnect) {
		// if (navigator.bluetooth) {
		// 	const options = { acceptAllDevices: true }
		// 	navigator.bluetooth
		// 		.requestDevice(options)
		// 		.then((device) => {
		// 			console.log("> Name:             " + device.name)
		// 			console.log("> Id:               " + device.id)
		// 			console.log("> Connected:        " + device.gatt.connected)
		// 		})
		// 		.catch((error) => {
		// 			console.log("Argh! " + error)
		// 		})
		// } else {
		alert("you are run in a slimulator")
		setTimeout(onconnect, 1000)
		// }
	},
	enable(x) {
		typeof x === "function" && x("OK")
	},
	isEnabled(x, xx) { setTimeout(() => x("OK"), 1000) },
	disconnect() { },
	write(a, s, c, str) {
		str = atob(str)
		if (str == "t") {
			window.test.count = 0
			window.test.interval = setInterval(() => {
				window.test.count++

				if (window.test.count >= 170) {
					clearInterval(window.test.interval)
					window.reader.glucose = "70"
					window.test.callback("end")
					console.log("end");
					return
				}

				window.test.callback("" + window.test.count + "," + Math.random(), "," + Math.random())
				console.log(window.test.count);
			}, 15)

		}
	},
	readUntil() { },
	startNotification(a, s, c, callback) {
		window.test.callback = callback
	},
	unsubscribe() { },
	list(x) {
		setTimeout(
			(r) =>
				x([
					{
						address: "98:D3:91:FD:B4:80",
						class: 7936,
						id: "98:D3:91:FD:B4:80",
						name: "WiTooth"
					},
					{
						address: "98:D3:91:FD:B4:81",
						class: 7936,
						id: "98:D3:91:FD:B4:81",
						name: "WiTooth 2"
					},
					{
						address: "98:D3:91:FD:B4:82",
						class: 7936,
						id: "98:D3:91:FD:B4:82",
						name: "WiTooth 3"
					}
				]),
			1000
		)
	},
	startStateNotifications(x) {
		// setTimeout(() => x("OK"), 1000)
	},
}

function bleService(address, service) {
	const device = {
		address: address,
		service: service,
		write: (char, data) => {
			console.log("write", char, data)
			window.blecen.write(this.address, this.service, char, data)
		},
		onsubscribe: (char, onread) => {
			console.log("onsubscribe", char, onread)
			window.blecen.subscribe(this.address, this.service, char, onread)
		},
		onunsubscribe: (char, onread) => {
			console.log("onunsubscribe", char, onread)
			window.blecen.unsubscribe(this.address, this.service, char)
		},
	}

	return device
}

const app1 = {
	ok(e) { console.log("ok", e) },
	error(e) { console.error("error", e) },
	warn(e) { console.warn("warning", e) },

	debug: false,
	timeoutId: 0,
	connected: false,
	bluetoothSerial: window.ble || mBLE1,
	enc: new TextDecoder("utf-8"),

	initialize: function () {
		if (this.debug) console.log("blecen.initialize:")
		document.addEventListener("deviceready", r => {
			if (this.debug) console.log("blecen.deviceready:")
			this.bluetoothSerial = window.ble || mBLE1
		}, false)
	},

	setPrioprity: function (address, priority) {
		/* Priority:
			0 - CONNECTION_PRIORITY_BALANCED
			1 - CONNECTION_PRIORITY_HIGH
			2 - CONNECTION_PRIORITY_LOW_POWER
		*/

		if (this.debug) console.log("blecen.setPrioprity:", address, priority)
		this.bluetoothSerial.requestConnectionPriority(address, priority)
	},

	scan: function (services, callback) {
		if (this.debug) console.log("blecen.list:")
		// this.setStatus("Looking for Bluetooth Devices...")
		// this.bluetoothSerial.list(callback, this.generateFailureFunction("List Failed"))

		if (!Array.isArray(services)) {
			services.trim().length == 0 && (services = [])
			services = [services]
		}

		this.bluetoothSerial.startScan(services, r => typeof callback === "function" && callback(r), this.error)
	},

	stop: function () {
		this.debug && console.log("blecen.stopScan:")
		this.bluetoothSerial.stopScan(this.ok, this.error)
	},

	list: function (callback) {
		this.debug && console.log("blecen.list:")
		this.bluetoothSerial.list(callback, this.error)
	},

	enabe(callback) {
		this.bluetoothSerial.enable(callback, this.error)
	},

	enable_connect(address, callback) {
		this.enable(() => {
			this.connect(address, callback)
		}, this.error)
	},

	connect: function (address, callback) {
		if (this.debug) console.log("blecen.connect:", address)
		console.log("Requesting connection to " + address)

		this.bluetoothSerial.isEnabled(r => {
			if (r == "OK") {
				this.bluetoothSerial.connect(address, r1 => {
					this.__onconnect(r1);
					typeof callback === "function" && callback(r1)
				}, this.error)
			} else {


				this.bluetoothSerial.enable()
			}
		}, r => enable_connect(address, callback))
	},

	disconnect: function (address) {
		this.debug && console.log("blecen.disconnect:", address)
		// this.setStatus("Disconnecting...")
		this.bluetoothSerial.disconnect(address, this.__ondisconnect, this.error)
	},

	__onconnect: function (r) {
		if (this.debug) console.log("blecen.onconnect:")
		// this.setStatus("Connected.")

		this.connected = true
		typeof this.onconnect === "function" && this.onconnect()


		this.bluetoothSerial.startStateNotifications(r => console.log(r), this.error)
	},

	__ondisconnect: function (r) {
		if (!this) return

		if (this.debug) console.log("blecen.ondisconnect:")
		// this.setStatus("Disconnected.")

		this.connected = false
		typeof this.ondisconnect === "function" && this.ondisconnect(r)
	},

	write: function (address, service, char, data) {
		if (this.debug) console.log("ble.write:", address, service, char, data)
		this.bluetoothSerial.write(address, service, char, btoa(data), this.ok, this.error)
	},

	subscribe: function (address, service, char, callback, asString = false) {
		this.debug && console.log("blecen.subscribe:", address, service, char)
		let new_callback = asString ? r => (typeof callback === "function" && callback(this.enc.decode(r))) : callback;

		this.bluetoothSerial.startNotification(address, service, char, new_callback, this.error)
	},

	unsubscribe: function (address, service, char) {
		if (this.debug) console.log("blecen.unsubscribe:", address, service, char)
		this.bluetoothSerial.stopNotification(address, service, char, this.ok, this.error)
	}
}

app1.initialize()
window.blecen = app1
