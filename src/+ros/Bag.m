classdef Bag
    % Bag A class for reading data from ROS bags.        
    properties
        handle = -1;
        cleanup = @(x) x;
    end

    methods
        function [obj] = Bag(bag)
            [~, ~, endian] = computer;
            if endian ~= 'L'
                error(['This machine is not little endian; ' ...
                       'rosbag_wrapper() won''t work']);
            end
            obj.handle = rosbag_wrapper(uint64(0), 'construct', bag);
            h = obj.handle;
            % Tell wrapper to destroy handle when we're deleted, but don't
            % generate an error if handle no longer exists
            % (e.g., due to clearing all envirnoment variables)
            obj.cleanup = onCleanup(@() rosbag_wrapper(uint64(0), 'destruct', h, false));
        end

        function [] = resetView(obj, topics)
        % resetView Reset which topics to read messages from
        %
        % resetView(topics) jumps to the beginning of the bagfile and changes
        % the view to be topics.  topics can be a string or a cell array of
        % strings.
            rosbag_wrapper(obj.handle, 'resetView', topics)
        end

        function [hn] = hasNext(obj)
        % hasNext Return true if there is at least one more message to read
            hn = rosbag_wrapper(obj.handle, 'hasNext');
        end

        function [msg, meta] = readMessage(obj)
        % readMessage Read a message from the bag
        % [MSG] = readMessage() gets the next message from the bag
        %
        % [MSG, META] = readMessage() gets the next message and return meta
        % data associated with it
            if nargout == 1
                msg = rosbag_wrapper(obj.handle, 'readMessage', false);
            else
                [msg, meta] = rosbag_wrapper(obj.handle, 'readMessage', true);
            end
        end

        function [msg, meta] = readAllMessages(obj, topics)
        % readAllMessages Read remaining messages from the bag
        % [MSG] = readAllMessages() returns all messages from the current
        % point on as a cell array.
        %
        % [MSG, META] = readAllMessages(...) returns meta data for each
        % message in a cell array.
        %
        % [...] = readAllMessages(topics) resets the view to 'topics' and
        % then reads all messages.
            if nargin == 2
                obj.resetView(topics)
            end
            if nargout == 1
                msg = rosbag_wrapper(obj.handle, 'readAllMessages', false);
            else
                [msg, meta] = rosbag_wrapper(obj.handle, 'readAllMessages', true);
            end
        end
    end
end