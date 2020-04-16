消息传递路径：
接收：
channel->network->peer(队列)->instance->role
发送：
role->peer->network->channel(队列)