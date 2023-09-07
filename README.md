# Транспортный справочник

Поддерживаются следующие функции:
- Возможность загрузки данных в формате JSON и генерации графического ответа в виде файла SVG, отображающего остановки и маршруты.
- Поиск оптимального маршрута между остановками.
- Для оптимизации вычислений используется сериализация справочной базы с помощью Google Protobuf.
- Внедрен конструктор JSON.
---
## Инструкция по запуску проекта (Clion)
Перейдите на [ официальный репозиторий Protocol Buffers на GitHub](https://github.com/protocolbuffers/protobuf/releases " официальный репозиторий Protocol Buffers на GitHub"), скачайте актуальную версию.

Откройте CLion, затем откройте папку с проектом или создайте новый проект по необходимости.

Перейдите к настройкам проекта, выбрав File -> Settings (или Preferences на macOS).

В разделе "Build, Execution, Deployment" выберите "CMake".

В поле "CMake options" добавьте следующие опции сборки:

`
-DCMAKE_BUILD_TYPE=Debug -Dprotobuf_BUILD_TESTS=OFF
`

Вы можете также добавить опцию 

`
-DCMAKE_INSTALL_PREFIX=<путь>
`

Выберите инструмент "Build" в CLion.
Затем выберите "Install" из выпадающего меню. Это запустит процесс установки Protocol Buffers на вашей системе.
В консоли вы увидите информацию о местоположении, в которое установлен Protocol Buffers. Обычно это будет папка с подпапками "bin", "include" и "lib".
для указания папки, в которую будет установлен protobuf. Путь может отличаться на разных операционных системах.

В поле "CMake options" добавьте следующую опцию, указав путь до установленного Protocol Buffers:

`
-DCMAKE_PREFIX_PATH=<путь_к_установленному_protobuf>
`

---
## Использование программы
Для создания базы общественного транспорта и ее сериализации в файл на основе запросов base_requests, запустите программу с параметром make_base, указав входной JSON-файл.
Пример запуска программы для создания базы:
`transport_catalogue.exe make_base <base.json>`

Чтобы использовать созданную базу и десериализовать ее для ответов на запросы, запустите программу с параметром process_requests, указав входной JSON-файл с запросами к базе и выходной файл, который будет содержать ответы на запросы.
Пример запуска программы для выполнения запросов к базе:
`transport_catalogue.exe process_requests <requests.json>out.txt`

---
## Формат входящих данных

Формал файла `base.json` должен иметь соответствующие ключи:
- `serialization_settings` - настройки сериализации.
- `routing_settings` - настройки маршрутизации.
- `render_settings` - настройки отрисовки.
- `base_requests` - массив данных об остановках и маршрутах.

Пример верно составленного запроса на построение базы:
<details>
  <summary>Пример корректного файла base.json:</summary>
```json
   {
      "serialization_settings": {
          "file": "transport_catalogue.db"
      },
      "routing_settings": {
          "bus_wait_time": 2,
          "bus_velocity": 30
      },
      "render_settings": {
          "width": 1200,
          "height": 500,
          "padding": 50,
          "stop_radius": 5,
          "line_width": 14,
          "bus_label_font_size": 20,
          "bus_label_offset": [
              7,
              15
          ],
          "stop_label_font_size": 18,
          "stop_label_offset": [
              7,
              -3
          ],
          "underlayer_color": [
              255,
              255,
              255,
              0.85
          ],
          "underlayer_width": 3,
          "color_palette": [
              "green",
              [
                  255,
                  160,
                  0
              ],
              "red"
          ]
      },
      "base_requests": [
          {
              "type": "Bus",
              "name": "14",
              "stops": [
                  "Улица Лизы Чайкиной",
                  "Электросети",
                  "Ривьерский мост",
                  "Гостиница Сочи",
                  "Кубанская улица",
                  "По требованию",
                  "Улица Докучаева",
                  "Улица Лизы Чайкиной"
              ],
              "is_roundtrip": true
          },
          {
              "type": "Bus",
              "name": "24",
              "stops": [
                  "Улица Докучаева",
                  "Параллельная улица",
                  "Электросети",
                  "Санаторий Родина"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Bus",
              "name": "114",
              "stops": [
                  "Морской вокзал",
                  "Ривьерский мост"
              ],
              "is_roundtrip": false
          },
          {
              "type": "Stop",
              "name": "Улица Лизы Чайкиной",
              "latitude": 43.590317,
              "longitude": 39.746833,
              "road_distances": {
                  "Электросети": 4300,
                  "Улица Докучаева": 2000
              }
          },
          {
              "type": "Stop",
              "name": "Морской вокзал",
              "latitude": 43.581969,
              "longitude": 39.719848,
              "road_distances": {
                  "Ривьерский мост": 850
              }
          },
          {
              "type": "Stop",
              "name": "Электросети",
              "latitude": 43.598701,
              "longitude": 39.730623,
              "road_distances": {
                  "Санаторий Родина": 4500,
                  "Параллельная улица": 1200,
                  "Ривьерский мост": 1900
              }
          },
          {
              "type": "Stop",
              "name": "Ривьерский мост",
              "latitude": 43.587795,
              "longitude": 39.716901,
              "road_distances": {
                  "Морской вокзал": 850,
                  "Гостиница Сочи": 1740
              }
          },
          {
              "type": "Stop",
              "name": "Гостиница Сочи",
              "latitude": 43.578079,
              "longitude": 39.728068,
              "road_distances": {
                  "Кубанская улица": 320
              }
          },
          {
              "type": "Stop",
              "name": "Кубанская улица",
              "latitude": 43.578509,
              "longitude": 39.730959,
              "road_distances": {
                  "По требованию": 370
              }
          },
          {
              "type": "Stop",
              "name": "По требованию",
              "latitude": 43.579285,
              "longitude": 39.733742,
              "road_distances": {
                  "Улица Докучаева": 600
              }
          },
          {
              "type": "Stop",
              "name": "Улица Докучаева",
              "latitude": 43.585586,
              "longitude": 39.733879,
              "road_distances": {
                  "Параллельная улица": 1100
              }
          },
          {
              "type": "Stop",
              "name": "Параллельная улица",
              "latitude": 43.590041,
              "longitude": 39.732886,
              "road_distances": {}
          },
          {
              "type": "Stop",
              "name": "Санаторий Родина",
              "latitude": 43.601202,
              "longitude": 39.715498,
              "road_distances": {}
          }
      ]
  }
```
</details>

