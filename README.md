# Справочник автобусных маршрутов
Ввод/вывод в формате JSON, рендеринг карты в SVG.
Применены собственные библиотеки, реализация однопоточная.
## Функции
- Поиск кратчайшего маршрута
- Вычисление времени маршрута
- Построение карты в SVG

# Сборка
cmake 3.x.x

# Использование
Чтение из stdin, вывод в stdout.

<details><summary>Пример ввода</summary>
<p>

```json
  {
      "base_requests": [
          {
              "is_roundtrip": true,
              "name": "297",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "Apteka",
                  "Biryulyovo Zapadnoye"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "635",
              "stops": [
                  "Biryulyovo Tovarnaya",
                  "Universam",
                  "Biryusinka",
                  "TETs 26",
                  "Pokrovskaya",
                  "Prazhskaya"
              ],
              "type": "Bus"
          },
          {
              "is_roundtrip": false,
              "name": "828",
              "stops": [
                  "Biryulyovo Zapadnoye",
                  "TETs 26",
                  "Biryusinka",
                  "Universam",
                  "Pokrovskaya",
                  "Rossoshanskaya ulitsa"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.574371,
              "longitude": 37.6517,
              "name": "Biryulyovo Zapadnoye",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 2600,
                  "TETs 26": 1100
              },
              "type": "Stop"
          },
          {
              "latitude": 55.587655,
              "longitude": 37.645687,
              "name": "Universam",
              "road_distances": {
                  "Biryulyovo Tovarnaya": 1380,
                  "Biryusinka": 760,
                  "Pokrovskaya": 2460
              },
              "type": "Stop"
          },
          {
              "latitude": 55.592028,
              "longitude": 37.653656,
              "name": "Biryulyovo Tovarnaya",
              "road_distances": {
                  "Universam": 890
              },
              "type": "Stop"
          },
          {
              "latitude": 55.581065,
              "longitude": 37.64839,
              "name": "Biryusinka",
              "road_distances": {
                  "Apteka": 210,
                  "TETs 26": 400
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580023,
              "longitude": 37.652296,
              "name": "Apteka",
              "road_distances": {
                  "Biryulyovo Zapadnoye": 1420
              },
              "type": "Stop"
          },
          {
              "latitude": 55.580685,
              "longitude": 37.642258,
              "name": "TETs 26",
              "road_distances": {
                  "Pokrovskaya": 2850
              },
              "type": "Stop"
          },
          {
              "latitude": 55.603601,
              "longitude": 37.635517,
              "name": "Pokrovskaya",
              "road_distances": {
                  "Rossoshanskaya ulitsa": 3140
              },
              "type": "Stop"
          },
          {
              "latitude": 55.595579,
              "longitude": 37.605757,
              "name": "Rossoshanskaya ulitsa",
              "road_distances": {
                  "Pokrovskaya": 3210
              },
              "type": "Stop"
          },
          {
              "latitude": 55.611717,
              "longitude": 37.603938,
              "name": "Prazhskaya",
              "road_distances": {
                  "Pokrovskaya": 2260
              },
              "type": "Stop"
          },
          {
              "is_roundtrip": false,
              "name": "750",
              "stops": [
                  "Tolstopaltsevo",
                  "Rasskazovka"
              ],
              "type": "Bus"
          },
          {
              "latitude": 55.611087,
              "longitude": 37.20829,
              "name": "Tolstopaltsevo",
              "road_distances": {
                  "Rasskazovka": 13800
              },
              "type": "Stop"
          },
          {
              "latitude": 55.632761,
              "longitude": 37.333324,
              "name": "Rasskazovka",
              "road_distances": {},
              "type": "Stop"
          }
      ],
      "render_settings": {
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ],
          "height": 200,
          "line_width": 14,
          "padding": 30,
          "stop_label_font_size": 20,
          "stop_label_offset": [
              7,
              -3
          ],
          "stop_radius": 5,
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "width": 200
      },
      "routing_settings": {
          "bus_velocity": 30,
          "bus_wait_time": 2
      },
      "stat_requests": [
          {
              "id": 1,
              "name": "297",
              "type": "Bus"
          },
          {
              "id": 2,
              "name": "635",
              "type": "Bus"
          },
          {
              "id": 3,
              "name": "828",
              "type": "Bus"
          },
          {
              "id": 4,
              "name": "Universam",
              "type": "Stop"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 5,
              "to": "Apteka",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 6,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 7,
              "to": "Pokrovskaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 8,
              "to": "Biryulyovo Zapadnoye",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Tovarnaya",
              "id": 9,
              "to": "Prazhskaya",
              "type": "Route"
          },
          {
              "from": "Apteka",
              "id": 10,
              "to": "Biryulyovo Tovarnaya",
              "type": "Route"
          },
          {
              "from": "Biryulyovo Zapadnoye",
              "id": 11,
              "to": "Tolstopaltsevo",
              "type": "Route"
          }
      ]
  }
  
```

</p>
</details>

<details><summary>Пример вывода</summary>
<p>

```json
  [
      {
          "curvature": 1.36159,
          "request_id": 1,
          "route_length": 5880,
          "stop_count": 6,
          "unique_stop_count": 5
      },
      {
          "curvature": 1.12195,
          "request_id": 2,
          "route_length": 14810,
          "stop_count": 11,
          "unique_stop_count": 6
      },
      {
          "curvature": 1.31245,
          "request_id": 3,
          "route_length": 15790,
          "stop_count": 11,
          "unique_stop_count": 6
      },
      {
          "buses": [
              "297",
              "635",
              "828"
          ],
          "request_id": 4
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 2,
                  "time": 3,
                  "type": "Bus"
              },
              {
                  "stop_name": "Biryusinka",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 0.42,
                  "type": "Bus"
              }
          ],
          "request_id": 5,
          "total_time": 7.42
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 4,
                  "time": 9.44,
                  "type": "Bus"
              }
          ],
          "request_id": 6,
          "total_time": 11.44
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 1.78,
                  "type": "Bus"
              },
              {
                  "stop_name": "Universam",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "828",
                  "span_count": 1,
                  "time": 4.92,
                  "type": "Bus"
              }
          ],
          "request_id": 7,
          "total_time": 10.7
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 4,
                  "time": 6.56,
                  "type": "Bus"
              }
          ],
          "request_id": 8,
          "total_time": 8.56
      },
      {
          "items": [
              {
                  "stop_name": "Biryulyovo Tovarnaya",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "635",
                  "span_count": 5,
                  "time": 14.32,
                  "type": "Bus"
              }
          ],
          "request_id": 9,
          "total_time": 16.32
      },
      {
          "items": [
              {
                  "stop_name": "Apteka",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 2.84,
                  "type": "Bus"
              },
              {
                  "stop_name": "Biryulyovo Zapadnoye",
                  "time": 2,
                  "type": "Wait"
              },
              {
                  "bus": "297",
                  "span_count": 1,
                  "time": 5.2,
                  "type": "Bus"
              }
          ],
          "request_id": 10,
          "total_time": 12.04
      },
      {
          "error_message": "not found",
          "request_id": 11
      }
  ]
  
  
```

</p>
</details>


 

