function s = openGeoComPort(nr, baud)

port = ['COM', num2str(nr)];

ports = instrfind;

openPorts = [];
for i=1:length(ports)
  if (strcmp(ports(i).Port, port))
    fclose(ports(i));
    delete(ports(i));
  elseif strcmp(ports(i).Status, 'open')
    openPorts = [openPorts, i]; %#ok<AGROW>
  end
end

s = serial(port, 'BaudRate', baud);
s.Terminator = 'CR/LF';
set(s, 'OutputBufferSize', 1024*4)
set(s, 'InputBufferSize', 1024*4)
set(s, 'Timeout', 10)

% close other open ports (due to bug in matlab)
for i = 1:length(openPorts)
  fclose(ports(openPorts(i)));
end

fopen(s);

% reopen other open ports (due to bug in matlab)
for i = 1:length(openPorts)
  fopen(ports(openPorts(i)));
end

%empty buffer
if s.BytesAvailable > 0
  fscanf(s, '%s', s.BytesAvailable);
end

