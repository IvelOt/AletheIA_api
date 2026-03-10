-- Aletheia Edge Stress Test Script Corrected
local selfie_path = "/home/levirenato/Faculdade/aletheia-api/Examples/foto_1.jpeg"
local doc_path = "/home/levirenato/Faculdade/aletheia-api/Examples/identidade_1.jpg"

local function read_file(path)
    local f = io.open(path, "rb")
    if not f then return "" end
    local content = f:read("*all")
    f:close()
    return content
end

local selfie_data = read_file(selfie_path)
local doc_data = read_file(doc_path)
local boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW"

-- Montagem manual do multipart garantindo escape de caracteres se necessário
-- Em Lua, strings binárias são seguras
local body = "--" .. boundary .. "\r\n" ..
             "Content-Disposition: form-data; name=\"selfie\"; filename=\"selfie.jpg\"\r\n" ..
             "Content-Type: image/jpeg\r\n\r\n" ..
             selfie_data .. "\r\n" ..
             "--" .. boundary .. "\r\n" ..
             "Content-Disposition: form-data; name=\"document\"; filename=\"doc.jpg\"\r\n" ..
             "Content-Type: image/jpeg\r\n\r\n" ..
             doc_data .. "\r\n" ..
             "--" .. boundary .. "--\r\n"

wrk.method = "POST"
wrk.body   = body
wrk.headers["Content-Type"] = "multipart/form-data; boundary=" .. boundary

function request()
   return wrk.format()
end
