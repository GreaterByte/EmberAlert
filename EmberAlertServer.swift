// By Theodor Faraon
// Version 10/7/2025

import Foundation

struct ServerData: Codeable {
    let humidity: Float
    let temperature_c: Float
    let temperature_f: Float
    let ir_raw: Float
    let ir_measured: Float
    let ir_estimated: Float
}


func fetchJSONData() {
    guard let url = URL(string: "http://127.0.0.1:5050/receive") else { //Change URL. This might not be the right server.
        print("Invalid url")
        return
    }

    let task = URLSession.shared.dataTask(with: url) { data, response, error in
        if let error = error{
        print("Error fetching data: \(error)")
            return
        }

        guard let httpResponse = response as? HTTPURLResponse,
            httpResponse.statusCode == 200 else {
                print("Invalid server response")
                return
            }
        
        guard let data = data else{
            print("No data received")
            return
        }

        // Decode JSON data into swfit construct
        do {
            let decodedData = try JSONDecoder().decode(ServerData.self, from: data)
            print("Received data: \(decodedData)")
        }
        catch {
            print("Failed to decode JSON : \(error)")
        }
    }

    task.resume()
}

fetchJSONData()