#!/usr/local/bin/perl -w
use IO::Socket;
use IO::Select;
	
my $acceptSocket = new IO::Socket::INET (
					LocalPort => '7070',
					Listen => 1,
					Reuse => 1,
				);
				
while ( my $client = $acceptSocket->accept() )
{
		my $server = new IO::Socket::INET(
			PeerAddr =>"192.168.66.163",
			PeerPort =>"5060",
		) or die "can not reach connect to tc_server $@";
		
		my $selector = new IO::Select($client, $server);
		my $CtoSbuffer = new IOBuffer( $server, "from SIP to setver" );
		my $StoCbuffer = new IOBuffer( $client, "from server to SIP" );
		
RL:		while ( 1 )	
		{
			foreach $s ($selector->can_read()) 
			{
				last RL unless ($s == $client ? $CtoSbuffer : $StoCbuffer)
						->ReadFromSocket( $s );					
			}
		}
		
		$server->close();
		$client->close();	
}

package IOBuffer;

sub new 
{
	my $class = shift;
	my $self = { 
		_data_buffer => "",
		_dest_socket => shift,
		_header_message => shift,
	};
	
	bless $self, $class;
};

sub ReadFromSocket
{
	my ($self, $s) = @_;
	my $tmp;
	return 0 unless ($s->recv($tmp,40,0) and $tmp);
	$self->{_data_buffer} .= $tmp;
	$self->private_CheckDataBuffer();
	return 1;
};

sub private_CheckDataBuffer
{
	my ($self) = @_;
	$_ = $self->{_data_buffer};
	return unless /\nContent-Length: *(\d+)/i;
	my $size = $1;
	return unless s/(^.*?\r\n\r\n.{$size})//s;
	$self->{_data_buffer} = $_;
	my $data = $1;	
	
	$data = $self->private_processSIPMessage_19420( $data );
	$self->private_Print( $data );
	$self->{_dest_socket}->send( $data );
};

sub private_Print
{
	my ($self, $data) = @_;
	print '-' x 60, "\n";
	print '  ' , $self->{_header_message} , "\n";
	print '-' x 60, "\n";
	print $data, "\n", '-' x 60, "\n\n\n\n";
}

sub  private_processSIPMessage_19420
{
	my ($self, $_) = @_;
	s/REGISTER (\S+) SIP\/2.0/REGISTER $1;transport=udp SIP\/2.0/i;
	return $_;
}

sub private_processSIPMessage_ACK_bug
{
	my ($self, $_) = @_;

	if ( /^INVITE/ )
	{
		s/Content-Length: *\d+/Content-Length: 0/i;
		s/\r\n\r\n(.+)$/\r\n\r\n/s;
		($self->{_sdp}) = $1;
		s/Content-Type: .*?\r\n//i;
	}
	
	if ( /^ACK/ )
	{
		my $size = length $self->{_sdp};
		s/Content-Length: (\d+)\r\n/Content-Length: $size\r\n/i;
		s/\r\n\r\n$/\r\nContent-Type: application\/sdp\r\n\r\n/i;
		$_ .= $self->{_sdp};
	}
	return $_;
}

