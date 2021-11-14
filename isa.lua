port = 32323

-- i set name of protocol as ISAMP as ISA Mail Protocol
isa_protocol = Proto("isamp","ISAMP Protocol")

-- finds next " where theres not \ infront of it in buffer_string (starting from arg_start) and retruns its index
function get_arg_end(arg_start, buffer_string)
    for i = arg_start, string.len(buffer_string) do
        if string.sub(buffer_string, i, i) == "\"" then
            if string.sub(buffer_string, i-1, i-1) == "\\" then
            else
                return i
            end
        end
    end
end

function isa_protocol.dissector(buffer,pinfo,tree)
    -- dont dissect empty packet as said in ws doc
    if  buffer:len() == 0 then return end

    -- find first word
    split_string = {};
    for match in (buffer():string():sub(2, -2).." "):gmatch("(.-)".." ") do
        table.insert(split_string, match);
    end

    -- i didnt implement reassemblying packets that were split into multiple so i ignore packets that dont have ) at end (those that arent whole)
    if buffer():string():sub(-1, -1) == ")" then
    else
        return 0
    end

    -- set type to response / request based on first word in data or dont dissect packet (ignoring those that were split)
    if split_string[1] == "ok" then
        status = "OK"
        type = "response"
    elseif split_string[1] == "err" then
        status = "ERROR"
        type = "response"
    elseif split_string[1] == "register" then
        type = "request"
        command = "register"
    elseif split_string[1] == "login" then
        type = "request"
        command = "login"
    elseif split_string[1] == "list" then
        type = "request"
        command = "list"
    elseif split_string[1] == "send" then
        type = "request"
        command = "send"
    elseif split_string[1] == "fetch" then
        type = "request"
        command = "fetch"
    elseif split_string[1] == "logout" then
        type = "request"
        command = "logout"
    else
        return 0;
    end

    -- sets Protocol column to ISAMP
    pinfo.cols.protocol = isa_protocol.name

    if type == "response" then
        -- sets where the message starts, if return code is OK then at 6th char if its ERR then its 7th char
        if status == "OK" then
            message_start = 6
        else 
            message_start = 7
        end

        -- gets message written by server
        message = buffer():string():sub(message_start, -3)

        -- create subtree for our protocol
        local subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")

        -- write basic info
        subtree:add("Raw message:", buffer():string())
        response = subtree:add("Type:", type)
        response:add("Status:", status)
        message_tree = response:add("Message:", message)

        -- looping through whole message with states and adding each message info as subtree
        -- these variables help us determine the state of automat
        in_arg = false
        in_message = false
        argsleft = 0
        findingid = false

        -- if its empty message dont loop through states
        if string.len(message) > 2 then
            -- for cycle looping through each character of emssage
            for i = 1, string.len(message) do
                -- if we are in concrete message ( we entered in '(' with some character before)
                if in_message == true then
                    -- if we are in " already we are looking for ending "
                    if in_arg == true then
                        if string.sub(message, i, i) == "\"" then
                            if string.sub(message, i-1, i-1) == "\\" then
                            else
                                -- we found " and char before is not \ 
                                -- now we are checking with argument this is ( if first one or second one) and then we are saving its ending index
                                if(argsleft == 2) then
                                    arg1_end = i
                                    argsleft = argsleft - 1
                                else
                                    arg2_end = i
                                    argsleft = argsleft - 1
                                end
                                in_arg = false
                            end
                        end
                        -- we are not in argument ", so we are look either for ending if message ')' or start of argument '"'
                    elseif in_arg == false then
                        -- we found ')' which is end of this message, we change state and save index of the end of message
                        if string.sub(message, i, i) == ")" then
                            in_message = false
                            end_of_message = i
                            -- we write out info about message if it was bigger than 2 chars (not sure if we can 
                            -- have second message of the list empty, prob not but just to makes sure i check it) and then write info about this message
                            if (end_of_message - start_of_message) > 2 then
                                submessage_tree = message_tree:add("Message" .. string.sub(message, message_id_start, message_id_end) .. ":")
                                submessage_tree:add("Sender:", string.sub(message, arg1_start, arg1_end))
                                submessage_tree:add("Subject:", string.sub(message, arg2_start, arg2_end))
                            end
                        end
                        -- we are not in argument and we found " so we are saving index of arg and setting the state
                        if string.sub(message, i, i) == "\"" then
                            if string.sub(message, i-1, i-1) == "\\" then
                            else
                                if argsleft == 2 then
                                    arg1_start = i
                                else
                                    arg2_start = i
                                end
                                in_arg = true
                            end
                        end
                        -- we just had ( and we are getting id till empty space so we are saving index when we meet empty space and then writing it at the end of message
                        if findingid == true then
                            if string.sub(message, i, i) == " " then
                                findingid = false
                                message_id_end = i-1
                            end
                        end                        
                    end
                -- we are not in message and we are looking for start of it and then setting variables to determine states
                elseif in_message == false then
                    if (string.sub(message, i, i) == "(") then
                        start_of_message = i
                        in_message = true
                        message_id_start = i+1
                        findingid = true
                        argsleft = 2
                    end
                end
            end
        end
        
        -- i wanted to show length of data at end so i add it here
        subtree:add("Length:", buffer:len())
    elseif type == "request" then
        -- i made different output for each command
        if command == "register" then
            -- setting start index of first argument
            -- " + ( + register + ' ' + "   = 12
            arg1_start = 12

            -- finds end of first argument and sets start of the second argument on 3 indexes further
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1
            arg2_start = returned_index + 3

            returned_index = get_arg_end(arg2_start, buffer():string())
            arg2_end = returned_index-1
            
            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Username:", buffer():string():sub(arg1_start, arg1_end))
            request:add("Password:", buffer():string():sub(arg2_start, arg2_end))
            subtree:add("Length:", buffer:len())
        elseif command == "login" then
            -- setting start index of first argument
            -- " + ( + login + ' ' + "   = 9
            -- finds starts and ends of arguments
            arg1_start = 9
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1
            arg2_start = returned_index + 3

            returned_index = get_arg_end(arg2_start, buffer():string())
            arg2_end = returned_index-1

            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Username:", buffer():string():sub(arg1_start, arg1_end))
            request:add("Password:", buffer():string():sub(arg2_start, arg2_end))
            subtree:add("Length:", buffer:len())
        elseif command == "send" then
            -- setting start index of first argument
            -- " + ( + send + ' ' + "   = 8
            arg1_start = 8
            -- finds starts and ends of arguments
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1
            arg2_start = returned_index + 3

            returned_index = get_arg_end(arg2_start, buffer():string())
            arg2_end = returned_index-1
            arg3_start = returned_index+3

            returned_index = get_arg_end(arg3_start, buffer():string())
            arg3_end = returned_index-1
            arg4_start = returned_index+3

            returned_index = get_arg_end(arg4_start, buffer():string())
            arg4_end = returned_index-1

            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Session_token:", buffer():string():sub(arg1_start, arg1_end))
            request:add("Recipient:", buffer():string():sub(arg2_start, arg2_end))
            request:add("Subject:", buffer():string():sub(arg3_start, arg3_end))
            request:add("Body:", buffer():string():sub(arg4_start, arg4_end))
            subtree:add("Length:", buffer:len())

        elseif command == "list" then
            -- setting start index of first argument
            -- " + ( + list + ' ' + "   = 8
            -- finds starts and ends of arguments
            arg1_start = 8
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1

            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Session_token:", buffer():string():sub(arg1_start, arg1_end))
            subtree:add("Length:", buffer:len())
        elseif command == "fetch" then
            -- setting start index of first argument
            -- " + ( + fetch + ' ' + "   = 8
            -- finds starts and ends of arguments
            arg1_start = 9
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1
            arg2_start = returned_index + 3        
            for i = arg2_start, string.len(buffer():string()) do
                if string.sub(buffer():string(), i, i) == ")" then
                    arg2_end = i-1
                    break
                end
            end

            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Session_token:", buffer():string():sub(arg1_start, arg1_end))
            request:add("Message ID:", buffer():string():sub(arg2_start, arg2_end))
            subtree:add("Length:", buffer:len())
        elseif command == "logout" then
            -- setting start index of first argument
            -- " + ( + logout + ' ' + "   = 10
            -- finds starts and ends of arguments
            arg1_start = 10
            returned_index = get_arg_end(arg1_start, buffer():string())
            arg1_end = returned_index - 1

            -- write info
            subtree = tree:add(isa_protocol, buffer(), "ISAMP Protocol Data")
            subtree:add("Raw message:", buffer():string())
            request = subtree:add("Type:", type)
            request:add("Command:", command)
            request:add("Session_token:", buffer():string():sub(arg1_start, arg1_end))
            subtree:add("Length:", buffer:len())
        end
    end
end

-- adding our dissector for 32323 tcp port
local tcp_port = DissectorTable.get("tcp.port")
tcp_port:add(port, isa_protocol)