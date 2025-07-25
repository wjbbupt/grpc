<?php
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# NO CHECKED-IN PROTOBUF GENCODE
# source: src/proto/grpc/testing/messages.proto

namespace Grpc\Testing;

use Google\Protobuf\Internal\GPBType;
use Google\Protobuf\Internal\RepeatedField;
use Google\Protobuf\Internal\GPBUtil;

/**
 * For reconnect interop test only.
 * Client tells server what reconnection parameters it used.
 *
 * Generated from protobuf message <code>grpc.testing.ReconnectParams</code>
 */
class ReconnectParams extends \Google\Protobuf\Internal\Message
{
    /**
     * Generated from protobuf field <code>int32 max_reconnect_backoff_ms = 1;</code>
     */
    protected $max_reconnect_backoff_ms = 0;

    /**
     * Constructor.
     *
     * @param array $data {
     *     Optional. Data for populating the Message object.
     *
     *     @type int $max_reconnect_backoff_ms
     * }
     */
    public function __construct($data = NULL) {
        \GPBMetadata\Src\Proto\Grpc\Testing\Messages::initOnce();
        parent::__construct($data);
    }

    /**
     * Generated from protobuf field <code>int32 max_reconnect_backoff_ms = 1;</code>
     * @return int
     */
    public function getMaxReconnectBackoffMs()
    {
        return $this->max_reconnect_backoff_ms;
    }

    /**
     * Generated from protobuf field <code>int32 max_reconnect_backoff_ms = 1;</code>
     * @param int $var
     * @return $this
     */
    public function setMaxReconnectBackoffMs($var)
    {
        GPBUtil::checkInt32($var);
        $this->max_reconnect_backoff_ms = $var;

        return $this;
    }

}

